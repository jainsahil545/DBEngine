#pragma once
#include <fstream>
#include <string>
#include "page.h"   // Your Page class header

class DiskManager {
public:
    // Constructor: accepts the file name/path, opens file in read/write mode (create if not exist)
    DiskManager(const std::string &fileName);

    // Destructor: flushes and closes the file
    ~DiskManager();

    // Read a page from disk into a Page object.
    // pageId is used to compute the offset (pageId * PAGE_SIZE)
    bool readPage(int pageId, Page &page);

    // Write a Page object to disk.
    // Serialize the Page into a raw buffer, then write at the appropriate offset.
    bool writePage(int pageId, const Page &page);

    // Allocate a new page by extending the file.
    // Returns the new pageId (for example, current number of pages)
    int allocateNewPage();

    // (Optional) Returns the current number of pages in the file.
    int getNumberOfPages() const;

private:
    std::string fileName_;
    std::fstream fileStream_;  // Use fstream for both input and output.
    int numPages_;             // Track the current number of pages in the file.
    
    // Helper method: computes file offset for a given pageId.
    long getOffset(int pageId) const {
        return static_cast<long>(pageId) * PAGE_SIZE;
    }
};
