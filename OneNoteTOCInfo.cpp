#include "OneNoteTOCInfo.h"

conote::OneNoteTOCInfo::OneNoteTOCInfo(const char* fileName)
{
	this->tocFileName = fileName;
}

conote::OneNoteTOCInfo::~OneNoteTOCInfo()
{
}

void conote::OneNoteTOCInfo::parseFile()
{
	uint64_t error;
	uint loopFlag = 1;
	
	this->tocFile = fopen(this->tocFileName.c_str(), "r");
	
	fseek(this->tocFile, 0, SEEK_END);
	cout << "File size: " << ftell(this->tocFile) << endl;
	fseek(this->tocFile, 0, SEEK_SET);
	
	if(this->tocFile == NULL)
		return;
		
	this->header = conote::OneStore::read_file_header(this->tocFile);
	OneStoreFileChunkReference64x32 rootFnlRef = this->header.fcrFileNodeListRoot;
	
//	json::serializer< ::OneStoreFileChunkReference64x32 >::serialize(std::cout, rootFnlRef);
	
	parse_frag(rootFnlRef.stp, rootFnlRef.cb);
	
	BOOST_LOG_TRIVIAL(debug) << "File parsed";

}

void conote::OneNoteTOCInfo::initialize()
{
	parseFile();
}

conote::OneNoteTOCInfo::OneNoteTOCInfo()
{
}

vector<string> conote::OneNoteTOCInfo::getAllSectionFileNames()
{

	
	
}

void conote::OneNoteTOCInfo::parse_frag(uint64_t fragStp, uint32_t fragCb)
{
	uint64_t error;
	OneStoreFileNodeListFragment frag;
	
	do {
	
		cout << "============  Frag START: " << fragStp << "===================" << endl;
		
//		for(int i = 0; i < this->visitedStps.size(); i++) {
//			if(this->visitedStps[i] == fragStp)
//				return;
//		}
//		
//		this->visitedStps.push_back(fragStp);
		
		frag = conote::OneStore::read_file_node_list_fragment(this->tocFile, fragStp, error);
		vector<OneStoreFileNode> fileNodes = conote::OneStore::get_file_nodes(this->tocFile, frag, fragStp, fragCb, true);
		
		cout << "============== Frag END: " << fragStp << "====================" << endl;
		
		for(int i = 0; i < fileNodes.size(); i++) {
			
			if(fileNodes[i].extra.refFlag) {
				cout << "Traversing to next file node list referred in file node: " << fileNodes[i].fnd.ref.stp << endl;
				parse_frag(fileNodes[i].fnd.ref.stp, fileNodes[i].fnd.ref.cb);
			}
			
			if(fileNodes[i].extra.otherRefFlag) {
				cout << "FileNode ID: " << std::hex << fileNodes[i].header.fileNodeID << endl;
				cout << "Reference to other structure in stream: " << std::dec << fileNodes[i].fnd.ref.stp << endl;
			}
			
			if(FILENODEID_OBJECTDECLARATIONWITHREFCOUNTFNDX == fileNodes[i].header.fileNodeID) {
				cout << "Object JCID: " << std::hex 
					<< conote::OneStore::jcid_to_uint32(conote::OneStore::get_jcid_by_jci(fileNodes[i].fnd.objectDeclarationWithRefCountFNDX.data.body.jci)) 
					<< std::dec << endl;
			}
			
			if(FILENODEID_OBJECTREVISIONWITHREFCOUNTFNDX == fileNodes[i].header.fileNodeID
				|| FILENODEID_OBJECTDECLARATIONWITHREFCOUNTFNDX == fileNodes[i].header.fileNodeID) {
				cout << "Getting the object space object prop set structure." << endl;
				OneStoreObjectSpaceObjectPropSet propSet;
				conote::onestore::PropSet propSetObj;
				OneStoreObjectSpaceObjectStreamHeader oidsHeader;
				vector<CompactID> oids, osids, ctxIds;
				
				oids = conote::OneStore::get_object_space_object_oids(this->tocFile, fileNodes[i].fnd.ref, propSet, oidsHeader);
				
				// this if block will be useful
				if(!oidsHeader.OsidStreamNotPresent) {
					OneStoreObjectSpaceObjectStreamHeader osidsHeader;
					osids = conote::OneStore::get_object_space_object_osids(this->tocFile, fileNodes[i].fnd.ref, propSet.extra.osidsOffset, propSet, osidsHeader);
					if(osidsHeader.ExtendedStreamsPresent) {
						OneStoreObjectSpaceObjectStreamHeader ctxIdsHeader;
						ctxIds = conote::OneStore::get_object_space_object_contextids(this->tocFile, fileNodes[i].fnd.ref, propSet.extra.contextIdsOffset, propSet, ctxIdsHeader);
					}
				}
				
				vector<OneStorePropertyID> prids;
				vector<conote::onestore::PropSetVal> rgData;
				size_t outPropsCount;
				// read the propset body
				conote::OneStore::get_propset_fields(this->tocFile, fileNodes[i].fnd.ref.stp, propSet.extra.propSetOffset, outPropsCount, prids, rgData, propSetObj, oids, osids, ctxIds);

			}
			
		}
		
		fragStp = frag.nextFragment.stp;
		
		if(frag.nextFragment.stp != SET_UINT64 && frag.nextFragment.cb != 0)
			cout << "Moving to next file node list referred: " << fragStp << endl;
	
	} while(frag.nextFragment.stp != SET_UINT64 && frag.nextFragment.cb != 0);
	
	cout << "End recursive function: parse_frag" << endl;
}
