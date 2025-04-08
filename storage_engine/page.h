#pragma once
#include <cstring>
#include <iostream>
#include <vector>
#include <cassert>
#define PAGE_SIZE 4096

// Slot entry for each record in the page.
struct Slot {
    int offset;   // Offset within the data area where the record begins.
    int length;   // Length of the record.
    bool isValid; // True if the slot contains a valid record.
};

// Page header structure holding fixed metadata.
struct PageHeader {
    int pageId;          // Unique identifier for the page.
    bool dirty;          // Indicates if the page has been modified.
    long lsn;            // Log Sequence Number (for WAL/recovery).
    int freeSpaceOffset; // Offset in the data array where record data ends.
    int numberOfSlots;   // Number of slot entries in the slot directory.
};

struct Page {
private:
    PageHeader header;   // Page metadata.
public:
    // The 'data' area will hold record data at the front and the slot directory at the end.
    // Total size is PAGE_SIZE - sizeof(PageHeader).
    char data[PAGE_SIZE - sizeof(PageHeader)];

    // In-memory representation of the slot directory.
    // This is not stored contiguously in memory with 'data' but is used when serializing/deserializing.
    std::vector<Slot> slotDirectory;

    // Constructor: Initialize header fields and clear the data area.
    Page() {
        header.pageId = -1;
        header.dirty = false;
        header.lsn = 0;
        header.freeSpaceOffset = 0;   // Initially, no record data is inserted.
        header.numberOfSlots = 0; 
        memset(data, 0, sizeof(data));
        slotDirectory.clear();
    }

    ~Page() {}

    // Accessor and mutator for pageId.
    int getPageId() const { return header.pageId; }
    void setPageId(int id) { header.pageId = id; }

    // Dirty flag accessors.
    bool isDirty() const { return header.dirty; }
    void makeDirty() { header.dirty = true; }

    // LSN accessors.
    long getLSN() const { return header.lsn; }
    void setLSN(long lsn) { header.lsn = lsn; }

    // Returns the free space available in the page.
    int getFreeSpace() const {
        // The slot directory is stored at the end of the data area.
        // Its starting offset is: (dataAreaSize - (numberOfSlots * sizeof(Slot)))
        int dataAreaSize = PAGE_SIZE - sizeof(PageHeader);
        int slotDirStart = dataAreaSize - (header.numberOfSlots * sizeof(Slot));
        return slotDirStart - header.freeSpaceOffset;
    }

    // Inserts a record into the page.
    // 'record' points to the record bytes and 'length' is the record size.
    // Returns the slot id on success, or -1 if there isn't enough space.
    int insertRecord(const char* record, int length) {
        // Calculate the space required: record data + a new slot entry.
        //so basically what we get from getFreeSpace is the space between the last record stored in the data section and slot directory vectors size
        int requiredSpace = length + sizeof(Slot);
        if (getFreeSpace() < requiredSpace) {
            return -1; // Not enough free space.
        }
        // Copy record data into the data area at freeSpaceOffset.
        memcpy(data + header.freeSpaceOffset, record, length);
        // Create a new slot entry.
        Slot newSlot;
        newSlot.offset = header.freeSpaceOffset;
        newSlot.length = length;
        newSlot.isValid = true;
        // Append the new slot to our in-memory slot directory.
        slotDirectory.push_back(newSlot);
        header.numberOfSlots++;
        // Update freeSpaceOffset.
        header.freeSpaceOffset += length;
        // Mark the page as dirty.
        header.dirty = true;
        // Return the slot id (i.e., the index of the new slot).
        return header.numberOfSlots - 1;
    }

    // Retrieves a record by slot id.
    // Copies the record into 'recordBuffer', which must be large enough.
    // Returns the length of the record, or -1 on error.
    int getRecord(int slotId, char* recordBuffer) const {
        if (slotId < 0 || slotId >= header.numberOfSlots)
            return -1;
        const Slot &slot = slotDirectory[slotId];
        if (!slot.isValid)
            return -1;
        memcpy(recordBuffer, data + slot.offset, slot.length);
        return slot.length;
    }

    // Deletes a record by slot id.
    bool deleteRecord(int slotId) {
        // Check for valid slot id.
        if (slotId < 0 || slotId >= header.numberOfSlots)
            return false;
        // Get the slot corresponding to the record to delete.
        Slot slotToDelete = slotDirectory[slotId];
        if (!slotToDelete.isValid)
            return false; // Already deleted.
        int recordOffset = slotToDelete.offset;
        int recordLength = slotToDelete.length;
        int freeSpaceEnd = header.freeSpaceOffset;
        // Calculate the number of bytes after the record.
        int bytesAfter = freeSpaceEnd - (recordOffset + recordLength);
        // If the record is not the last in the data area, shift subsequent data left.
        if (bytesAfter > 0) {
            // Move data from the end of the deleted record to the start position.
            memmove(data + recordOffset, data + recordOffset + recordLength, bytesAfter);
        }
        // Update the free space pointer.
        header.freeSpaceOffset -= recordLength;
        // Remove the deleted slot from the slot directory.
        slotDirectory.erase(slotDirectory.begin() + slotId);
        header.numberOfSlots--;
        // Update the offsets for all slots that follow the deleted record.
        for (int i = slotId; i < header.numberOfSlots; ++i) {
            slotDirectory[i].offset -= recordLength;
        }
        // Mark the page as dirty since it has been modified.
        header.dirty = true;
        return true;
    }
    

    // Serializes the page into a raw buffer of PAGE_SIZE bytes.
    // The layout is:
    // [Header][Record Data (from 0 to freeSpaceOffset)][Unused Space][Slot Directory (packed at the end)]
    void serialize(char* buffer) const {
        // 1. Copy the header.
        memcpy(buffer, &header, sizeof(header));
        // 2. Copy the record data.
        // We copy only up to freeSpaceOffset (the region actually used for records).
        memcpy(buffer + sizeof(header), data, header.freeSpaceOffset);
        // 3. Pack the slot directory at the end of the data area.
        int dataAreaSize = PAGE_SIZE - sizeof(PageHeader);
        int slotDirSize = header.numberOfSlots * sizeof(Slot);
        int slotDirStart = dataAreaSize - slotDirSize;
        // Copy the slot directory from our in-memory vector into the appropriate location in the buffer.
        memcpy(buffer + sizeof(header) + slotDirStart, slotDirectory.data(), slotDirSize);
    }

    // Deserializes a raw buffer (of PAGE_SIZE bytes) into the Page object.
    void deserialize(const char* buffer) {
        memcpy(&header, buffer, sizeof(header));
        // Copy record data.
        memcpy(data, buffer + sizeof(header), header.freeSpaceOffset);
        // Retrieve the slot directory.
        int slotDirSize = header.numberOfSlots * sizeof(Slot);
        int dataAreaSize = PAGE_SIZE - sizeof(PageHeader);
        int slotDirStart = dataAreaSize - slotDirSize;
        slotDirectory.resize(header.numberOfSlots);
        memcpy(slotDirectory.data(), buffer + sizeof(header) + slotDirStart, slotDirSize);
    }

    // Clears the page.
    void clear() {
        header.pageId = -1;
        header.dirty = false;
        header.lsn = 0;
        header.freeSpaceOffset = 0;
        header.numberOfSlots = 0;
        memset(data, 0, sizeof(data));
        slotDirectory.clear();
    }

    // Prints page header and slot directory information for debugging.
    void printInfo() const {
        std::cout << "Page ID: " << header.pageId << "\n"
                  << "Dirty: " << (header.dirty ? "true" : "false") << "\n"
                  << "LSN: " << header.lsn << "\n"
                  << "Free Space Offset: " << header.freeSpaceOffset << "\n"
                  << "Number of Slots: " << header.numberOfSlots << "\n"
                  << "Free Space Available: " << getFreeSpace() << "\n";
        std::cout << "Slot Directory:\n";
        for (int i = 0; i < header.numberOfSlots; ++i) {
            const Slot &s = slotDirectory[i];
            std::cout << "  Slot " << i << ": offset=" << s.offset
                      << ", length=" << s.length
                      << ", valid=" << (s.isValid ? "true" : "false") << "\n";
        }
    }
};
