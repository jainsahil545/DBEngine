#include "diskmanager.h"

DiskManager::DiskManager(const std::string& fileName) : fileName_(fileName) {
    // Open the file in read/write mode (binary)
    fileStream_.open(fileName_, std::ios::in | std::ios::out | std::ios::binary);
    if (!fileStream_.is_open()) {
        // If opening failed, create the file.
        fileStream_.clear();
        fileStream_.open(fileName_, std::ios::out | std::ios::binary);
        fileStream_.close();
        // Re-open in read/write mode.
        fileStream_.open(fileName_, std::ios::in | std::ios::out | std::ios::binary);
    }
    // Compute file size by seeking to the end.
    fileStream_.seekg(0, std::ios::end);
    std::streampos fileSize = fileStream_.tellg();
    // Compute number of pages.
    numPages_ = static_cast<int>(fileSize / PAGE_SIZE);
    // Reset read position.
    fileStream_.seekg(0, std::ios::beg);
}

DiskManager::~DiskManager() {
    fileStream_.close();
}

bool DiskManager::readPage(int pageId, Page &page) {
    int offset = pageId * PAGE_SIZE;
    fileStream_.seekg(offset, std::ios::beg);
    if (!fileStream_) {
        return false;
    }
    char buffer[PAGE_SIZE];
    fileStream_.read(buffer, PAGE_SIZE);
    if (fileStream_.gcount() != PAGE_SIZE) {
        return false;
    }
    page.deserialize(buffer);
    return true;
}

bool DiskManager::writePage(int pageId, const Page &page) {
    int offset = 0;
    if (pageId == numPages_) {
        offset = numPages_ * PAGE_SIZE;
    } else if (pageId < numPages_) {
        offset = pageId * PAGE_SIZE;
    } else {
        return false;
    }
    fileStream_.seekp(offset, std::ios::beg);
    if (!fileStream_) {
        return false;
    }
    char buffer[PAGE_SIZE];
    page.serialize(buffer);
    fileStream_.write(buffer, PAGE_SIZE);
    if (!fileStream_) {
        return false;
    }
    fileStream_.flush();
    if (pageId == numPages_) {
        numPages_++;
    }
    return true;
}

int DiskManager::allocateNewPage() {
    // Create a new empty page.
    Page newPage;
    // Set its page id (optional, depending on your Page design)
    newPage.setPageId(numPages_);
    // Write the new page to the end of the file.
    if (writePage(numPages_, newPage)) {
        return numPages_ - 1; // writePage increments numPages_ on success.
    }
    return -1; // Return -1 to indicate failure.
}

int DiskManager::getNumberOfPages() const {
    return numPages_;
}
