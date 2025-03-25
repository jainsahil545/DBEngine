# DBEngine

This project is a lightweight, single-node, disk-based relational database designed for a single user. It is built from scratch in C++ and follows a layered architecture that mimics modern database systems. The project is organized into distinct components that work together to provide efficient, robust, and modular database functionality.

Project Overview
The database system includes the following key components:

Page Management:
Implements a slotted page design to manage fixed-size pages on disk. Each page consists of a header (with metadata such as page ID, dirty flag, LSN, free-space offset, and slot count) and a data area. The data area is divided between record storage (growing from the beginning) and a slot directory (growing from the end) that keeps track of the location and size of each record.

Disk Manager:
Handles low-level file I/O operations, including reading and writing entire pages to/from a disk file. The Disk Manager supports random-access operations by computing offsets based on page IDs and a fixed page size. It also ensures durability by flushing data to disk as needed.

Buffer Pool Manager:
Acts as a cache layer between the Disk Manager and higher-level components. It manages an in-memory buffer pool of fixed-size frames, each of which holds a page. The Buffer Pool Manager provides mechanisms for fixing (pinning) and unfixing (unpinning) pages, employs an LRU-based eviction policy, and maintains a page table mapping page IDs to frame indices.

Heap File Manager (Upcoming):
Will serve as a record-level interface built on top of the Buffer Pool Manager. It will handle operations such as inserting, retrieving, updating, and deleting records within pages. The Heap File Manager will leverage the slotted page design to efficiently manage free space and the slot directory.

Key Features
Modular Architecture:
The project is divided into well-defined layers (Page, Disk Manager, Buffer Pool) to isolate functionality and simplify maintenance and future expansion.

Slotted Page Design:
Pages are organized using a slotted page layout, which supports variable-length records, efficient space management, and record-level operations.

Efficient Disk I/O:
The Disk Manager provides fast random-access operations on a disk file by computing offsets using a fixed page size.

Buffer Pool with LRU Eviction:
The Buffer Pool caches frequently accessed pages in memory, reducing disk I/O. It uses an LRU (Least Recently Used) policy, along with pin counts, to determine which pages to evict when the buffer is full.

Single-User, Local Relational Database:
The database is designed for a single user on a local machine, making it an excellent educational project and a starting point for more advanced multi-user, distributed systems.

Future Enhancements
Heap File Manager and Query Execution Engine:
The next steps include implementing a Heap File Manager for record-level operations and a simple SQL-like query parser and execution engine to interact with the data.

Indexing:
Adding support for indexes (e.g., B+ trees) to accelerate query processing and record lookups.

Multi-threading and Concurrency Control:
Introducing multi-threading support and concurrency control mechanisms (e.g., locks, transactions) to scale the database for multiple users.

Getting Started
Clone the Repository:

bash
Copy
git clone https://github.com/yourusername/advanced-relational-db.git
cd advanced-relational-db
Build the Project: Use your preferred C++ build system (e.g., CMake, Makefiles). For example, with a simple Makefile:

bash
Copy
make
Run the Tests/Examples: Run the provided test programs to see the database components in action.

bash
Copy
./runTests
Contributing
Contributions, suggestions, and bug reports are welcome. Feel free to open issues or pull requests to help improve the project.

