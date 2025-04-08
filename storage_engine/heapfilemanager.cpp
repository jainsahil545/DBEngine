#include "heapfilemanager.h"

HeapFile::HeapFile(BufferPool* bp): bp(bp_)
{
	
}

HeapFile::~HeapFile()
{
	
}

RecordId RecordId::insertRecord(std::vector<char>& record)
{
	//find the free page
	//fix the page
	//Insert the record
	//unfix the page	
}

bool RecordId::deleteRecord(RecordId rid)
{
	
}

bool RecordId::getRecord(RecordId rid, std::vector<char>& record)
{
	
}

bool RecordId::updateRecord(RecordId rid, const std::vector<char> record)
{
	
}
