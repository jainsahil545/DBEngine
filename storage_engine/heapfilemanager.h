#include <vector>
#include <iostream>
#include <unordered_map>

struct RecordId
{
	int pageId;
	int slotId;
};

class HeapFile
{
public:
	HeapFile(BufferPool* bp);

	~HeapFile();

	RecordId insertRecord(std::vector<char>& record);

	bool deleteRecord(RecordId rid);

	bool getRecord(RecordId rid, std::vector<char>& record);

	bool updateRecord(RecordId rid, const std::vector<char>& record);
	
private:
	BufferPool* bp_;
	//pageid to Number of bytes availabe in the map.
	std::unordered_map<int,int> freeSpaceMap_;	
};
