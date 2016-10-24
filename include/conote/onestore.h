#ifndef MS_ONESTORE_H
#define MS_ONESTORE_H


#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

#include "conote/types.h"

#ifdef __cplusplus
extern "C" {
#endif










/* File Node Ref structures */
typedef struct {
	// Stream pointer
	uint64_t stp;
	// Size
	uint64_t cb;
}OneStoreFileChunkReference64;

typedef struct {
	uint32_t stp;
	uint32_t cb;
}OneStoreFileChunkReference32;

typedef struct _OneStoreFileChunkReference64x32{
	uint64_t stp __attribute__((packed));
	uint32_t cb;
}OneStoreFileChunkReference64x32;

typedef struct _OneStoreFileNodeChunkReference {
	uint64_t stp;
	uint64_t cb;
} OneStoreFileNodeChunkReference;

static const OneStoreFileNodeChunkReference fcrNil = { SET_UINT64, 0 };
static const OneStoreFileChunkReference64x32 fcrNil64x32 = { SET_UINT64, 0 };
static const OneStoreFileNodeChunkReference fcrZero = { 0, 0 };


/* Other common structures */

typedef struct _JCID {
	uint16_t index;
	unsigned int IsBinary : 1;
	unsigned int IsPropertySet : 1;
	unsigned int IsGraphNode : 1;
	unsigned int IsFileData : 1;
	unsigned int IsReadOnly : 1;
	unsigned int Reserved : 11;
} __attribute__((packed)) JCID;











#define ONESTORE_FILE_HEADER_SIZE 1024

/* The file header of .ONE and .ONETOC2 files. Refer the specification for details.
 * The header size is 1024 bytes.
 */ 
typedef struct {
	GUID guidFileType;
	GUID guidFile;
	GUID guidLegacyFileVersion;
	GUID guidFileFormat;
	uint32_t ffvLastCodeThatWroteToThisFile;
	uint32_t ffvOldestCodeThatHasWrittenToThisFile;
	uint32_t ffvNewestCodeThatHasWrittenToThisFile;
	uint32_t ffvOldestCodeThatMayReadThisFile;
	OneStoreFileChunkReference32 fcrLegacyFreeChunkList;
	OneStoreFileChunkReference32 fcrLegacyTransactionLog;
	uint32_t cTransactionsInLog;
	uint32_t cbLegacyExpectedFileLength;
	uint64_t rgbPlaceholder;
	OneStoreFileChunkReference32 fcrLegacyFileNodeListRoot;
	uint32_t cbLegacyFreeSpaceInFreeChunkList;
	uint8_t fNeedsDefrag;
	uint8_t fRepairedFile;
	uint8_t fNeedsGarbageCollect;
	uint8_t fHasNoEmbeddedFileObjects;
	GUID guidAncestor;
	uint32_t crcName;
	OneStoreFileChunkReference64x32 fcrHashedChunkList;
	OneStoreFileChunkReference64x32 fcrTransactionLog;
	OneStoreFileChunkReference64x32 fcrFileNodeListRoot;
	OneStoreFileChunkReference64x32 fcrFreeChunkList;
	uint64_t cbExpectedFileLength;
	uint64_t cbFreeSpaceInFreeChunkList;
	GUID guidFileVersion;
	uint64_t nFileVersionGeneration;
	GUID guidDenyReadFileVersion;
	uint32_t grfDebugLogFlags;
	OneStoreFileChunkReference64x32 fcrDebugLog;
	OneStoreFileChunkReference64x32 fcrAllocVerificationFreeChunkList;
	uint32_t bnCreated;
	uint32_t bnLastWroteToThisFile;
	uint32_t bnOldestWritten;
	uint32_t bnNewestWritten;
	uint8_t rgbReserved[728];
} __attribute__((packed)) OneStoreFileHeaderRaw;











/* Structures refering only FileNodeChunkRef */

typedef struct _OneStoreFileNodeChunkRefContain {
	OneStoreFileNodeChunkReference ref;
} OneStoreRevisionManifestListReferenceFND,
  OneStoreObjectDataEncryptionKeyV2FNDX, // refers OneStoreDataEncryptionKey (below)
  OneStoreFileDataStoreListReferenceFND;






/* Free Chunk List */



/* The location of the first FreeChunkListFragment structure is specified by a Header.fcrFreeChunkList */
typedef struct {
	uint32_t crc;
	uint8_t fcrNextChunk[16];

	/* fcrFreeChunk (variable): An array of FileChunkReference64 structures (section 2.2.4.3) 
	 * where each element in the array specifies a reference to unused parts of the file. 
	 * The number of elements is given by (cb – 16) / 16 where cb is the size, in bytes, of this FreeChunkListFragment structure; 
	 * cb is specified by the file chunk reference (section 2.2.4) that references this FreeChunkListFragment structure. */
	uint8_t* fcrFreeChunk;

} OneStoreFreeChunkListFragmentRaw;





/* File Node */

#define FILENODEID_OBJECTSPACEMANIFESTLISTREFERENCEFND 0x008
#define FILENODEID_OBJECTSPACEMANIFESTROOTFND 0x004
#define FILENODEID_OBJECTSPACEMANIFESTLISTSTARTFND 					0x00C
#define FILENODEID_REVISIONMANIFESTLISTREFERENCEFND 					0x010
#define FILENODEID_REVISIONMANIFESTLISTSTARTFND 						0x014
#define FILENODEID_REVISIONMANIFESTSTART4FND 							0x01B
#define FILENODEID_REVISIONMANIFESTENDFND 								0x01C
#define FILENODEID_REVISIONMANIFESTSTART6FND 							0x01E
#define FILENODEID_REVISIONMANIFESTSTART7FND 							0x01F
#define FILENODEID_GLOBALIDTABLESTARTFNDX 								0x021
#define FILENODEID_GLOBALIDTABLESTART2FND 								0x022
#define FILENODEID_GLOBALIDTABLEENTRYFNDX 								0x024
#define FILENODEID_GLOBALIDTABLEENTRY2FNDX 							0x025
#define FILENODEID_GLOBALIDTABLEENTRY3FNDX 							0x026
#define FILENODEID_GLOBALIDTABLEENDFNDX 								0x028
#define FILENODEID_OBJECTDECLARATIONWITHREFCOUNTFNDX 					0x02D
#define FILENODEID_OBJECTDECLARATIONWITHREFCOUNT2FNDX 					0x02E
#define FILENODEID_OBJECTREVISIONWITHREFCOUNTFNDX 						0x041
#define FILENODEID_OBJECTREVISIONWITHREFCOUNT2FNDX 					0x042
#define FILENODEID_ROOTOBJECTREFERENCE2FNDX 							0x059
#define FILENODEID_ROOTOBJECTREFERENCE3FND 							0x05A
#define FILENODEID_REVISIONROLEDECLARATIONFND 							0x05C
#define FILENODEID_REVISIONROLEANDCONTEXTDECLARATIONFND 				0x05D
#define FILENODEID_OBJECTDECLARATIONFILEDATA3REFCOUNTFND 				0x072
#define FILENODEID_OBJECTDECLARATIONFILEDATA3LARGEREFCOUNTFND 			0x073
#define FILENODEID_OBJECTDATAENCRYPTIONKEYV2FNDX 						0x07C
#define FILENODEID_OBJECTINFODEPENDENCYOVERRIDESFND 					0x084
#define FILENODEID_DATASIGNATUREGROUPDEFINITIONFND 					0x08C
#define FILENODEID_FILEDATASTORELISTREFERENCEFND 						0x090
#define FILENODEID_FILEDATASTOREOBJECTREFERENCEFND 					0x094
#define FILENODEID_OBJECTDECLARATION2REFCOUNTFND 						0x0A4
#define FILENODEID_OBJECTDECLARATION2LARGEREFCOUNTFND 					0x0A5
#define FILENODEID_OBJECTGROUPLISTREFERENCEFND 						0x0B0
#define FILENODEID_OBJECTGROUPSTARTFND 								0x0B4
#define FILENODEID_OBJECTGROUPENDFND 									0x0B8
#define FILENODEID_HASHEDCHUNKDESCRIPTOR2FND 							0x0C2
#define FILENODEID_READONLYOBJECTDECLARATION2REFCOUNTFND 				0x0C4
#define FILENODEID_READONLYOBJECTDECLARATION2LARGEREFCOUNTFND 			0x0C5
#define FILENODEID_CHUNKTERMINATORFND 0x0FF

#define FILENODE_CBFORMAT_UNCOMPRESSED_4_BYTES 0
#define FILENODE_CBFORMAT_UNCOMPRESSED_8_BYTES 1
#define FILENODE_CBFORMAT_COMPRESSED_1_BYTE 2
#define FILENODE_CBFORMAT_COMPRESSED_2_BYTES 3

#define FILENODE_STPFORMAT_UNCOMPRESSED_8_BYTES 0
#define FILENODE_STPFORMAT_UNCOMPRESSED_4_BYTES 1
#define FILENODE_STPFORMAT_COMPRESSED_2_BYTES 2
#define FILENODE_STPFORMAT_COMPRESSED_4_BYTES 3

#define FILENODE_BASETYPE_NO_REF 0
#define FILENODE_BASETYPE_REF 1
#define FILENODE_BASETYPE_REF_FILE_NODE_LIST 2

/* Property IDs */
#define PROPERTYID_NODATA 0x1
#define PROPERTYID_BOOL 0x2
#define PROPERTYID_ONEBYTEOFDATA 0x3
#define PROPERTYID_TWOBYTESOFDATA 0x4
#define PROPERTYID_FOURBYTESOFDATA 0x5
#define PROPERTYID_EIGHTBYTESOFDATA 0x6
#define PROPERTYID_FOURBYTESOFLENGTHFOLLOWEDBYDATA 0x7
#define PROPERTYID_OBJECTID 0x8
#define PROPERTYID_ARRAYOFOBJECTIDS 0x9
#define PROPERTYID_OBJECTSPACEID 0xA
#define PROPERTYID_ARRAYOFOBJECTSPACEIDS 0xB
#define PROPERTYID_CONTEXTID 0xC
#define PROPERTYID_ARRAYOFCONTEXTIDS 0xD
#define PROPERTYID_ARRAYOFPROPERTYVALUES 0x10
#define PROPERTYID_PROPERTYSET 0x11

/* this is part of the FileNode structure */
typedef struct _OneStoreFileNodeHeader {
	unsigned int fileNodeID : 10 __attribute__((packed));
	unsigned int Size : 13  __attribute__((packed));
	unsigned int aStpFormat : 2 __attribute__((packed));
	unsigned int bCbFormat : 2  __attribute__((packed));
	unsigned int cBaseType : 4  __attribute__((packed));
	unsigned int dReserved : 1;
} OneStoreFileNodeHeader;


/* File Node Types */

typedef struct _OneStoreObjectSpaceManifestRootFND {
	ExtendedGUID gosidRoot;
} OneStoreObjectSpaceManifestRootFND;

typedef struct _OneStoreObjectSpaceManifestListReferenceFND {
	OneStoreFileNodeChunkReference ref;
	ExtendedGUID gosid;
} OneStoreObjectSpaceManifestListReferenceFND;

typedef struct _OneStoreObjectSpaceManifestListStartFND {
	ExtendedGUID gosid;
} OneStoreObjectSpaceManifestListStartFND;

typedef struct _OneStoreRevisionManifestListStartFND {
	ExtendedGUID gosid;
	uint32_t nInstance;
} __attribute__((packed)) OneStoreRevisionManifestListStartFND;

typedef struct _OneStoreRevisionManifestStart4FND {
	ExtendedGUID rid;
	ExtendedGUID ridDependent;
	uint64_t timeCreation;
	int32_t RevisionRole;
	uint16_t odsDefault;
} __attribute__((packed)) OneStoreRevisionManifestStart4FND;

typedef struct _OneStoreRevisionManifestStart6FND {
	ExtendedGUID rid;
	ExtendedGUID ridDependent;
	int32_t RevisionRole;
	uint16_t odsDefault;
} __attribute__((packed)) OneStoreRevisionManifestStart6FND;

typedef struct _OneStoreRevisionManifestStart7FND {
	OneStoreRevisionManifestStart6FND base;
	ExtendedGUID gctxid;
} __attribute__((packed)) OneStoreRevisionManifestStart7FND;

typedef struct _OneStoreGlobalIdTableStartFNDX {
	uint8_t Reserved;
} __attribute__((packed)) OneStoreGlobalIdTableStartFNDX;


typedef struct _OneStoreGlobalIdTableEntryFNDX {
	uint32_t index;
	GUID guid;
} __attribute__((packed)) OneStoreGlobalIdTableEntryFNDX;

typedef struct _OneStoreGlobalIdTableEntry2FNDX {
	uint32_t iIndexMapFrom;
	uint32_t iIndexMapTo;
} OneStoreGlobalIdTableEntry2FNDX;

typedef struct _OneStoreGlobalIdTableEntry3FNDX {
	uint32_t iIndexCopyFromStart;
	uint32_t cEntriesToCopy;
	uint32_t iIndexCopyToStart;
} __attribute__((packed)) OneStoreGlobalIdTableEntry3FNDX;

typedef struct _OneStoreObjectRevisionWithRefCountFNDXData {
	CompactID oid;
	unsigned int fHasOidReferences : 1;
	unsigned int fHasOsidReferences : 1;
	unsigned int cRef : 6;
} __attribute__((packed)) OneStoreObjectRevisionWithRefCountFNDXData;

typedef struct _OneStoreObjectRevisionWithRefCountFNDX {
	OneStoreFileNodeChunkReference ref;
	OneStoreObjectRevisionWithRefCountFNDXData data;
} OneStoreObjectRevisionWithRefCountFNDX;

typedef struct _OneStoreObjectRevisionWithRefCount2FNDXData {
	CompactID oid;
	unsigned int fHasOidReferences : 1;
	unsigned int fHasOsidReferences : 1;
	uint32_t Reserved : 30;
	uint32_t cRef : 4;
} __attribute__((packed)) OneStoreObjectRevisionWithRefCount2FNDXData;

typedef struct _OneStoreObjectRevisionWithRefCount2FNDX {
	OneStoreFileNodeChunkReference ref;
	OneStoreObjectRevisionWithRefCount2FNDXData data;
} OneStoreObjectRevisionWithRef2CountFNDX;

typedef struct _OneStoreRootObjectReference2FNDX {
	CompactID oid;
	uint32_t RootRole;
} OneStoreRootObjectReference2FNDX;

typedef struct _OneStoreRootObjectReference3FND {
	ExtendedGUID oidRoot;
	uint32_t RootRole;
} __attribute__((packed)) OneStoreRootObjectReference3FND;

typedef struct _OneStoreRevisionRoleDeclarationFND {
	ExtendedGUID rid;
	uint32_t RevisionRole;
} __attribute__((packed)) OneStoreRevisionRoleDeclarationFND;

typedef struct _OneStoreRevisionRoleAndContextDeclarationFND {
	OneStoreRevisionRoleDeclarationFND base;
	ExtendedGUID gctxid;
} __attribute__((packed)) OneStoreRevisionRoleAndContextDeclarationFND;

typedef struct _OneStoreDataEncryptionKeyV2 {
	// must be 0xFB6BA385DAD1A067.
	uint64_t Header;
	uint8_t* EncryptionData;
	// must be 0x2649294F8E198B3C.
	uint64_t Footer;
} OneStoreDataEncryptionKeyV2;

typedef struct _OneStoreObjectInfoDependencyOverride8 {
	CompactID oid;
	uint8_t cRef;
} __attribute__((packed)) OneStoreObjectInfoDependencyOverride8;

typedef struct _OneStoreObjectInfoDependencyOverride32 {
	CompactID oid;
	uint32_t cRef;
} __attribute__((packed)) OneStoreObjectInfoDependencyOverride32;

typedef struct _OneStoreObjectInfoDependencyOverrideHeader {
	uint32_t c8BitOverrides;
	uint32_t c32BitOverrides;
	uint32_t crc;
} __attribute__((packed)) OneStoreObjectInfoDependencyOverrideHeader;

typedef struct _OneStoreObjectInfoDependencyOverrideData {
	OneStoreObjectInfoDependencyOverrideHeader header;
	// array of overrides; dont change back to normal struct member.
	OneStoreObjectInfoDependencyOverride8* Overrides1;
	OneStoreObjectInfoDependencyOverride32* Overrides2;
} __attribute__((packed)) OneStoreObjectInfoDependencyOverrideData;

typedef struct _OneStoreObjectInfoDependencyOverridesFND {
	OneStoreFileNodeChunkReference ref;
	OneStoreObjectInfoDependencyOverrideData data;
} OneStoreObjectInfoDependencyOverridesFND;

typedef struct _OneStoreFileDataStoreObjectReferenceFND {
	OneStoreFileNodeChunkReference ref;
	GUID guidReference;
} OneStoreFileDataStoreObjectReferenceFND;

typedef struct _OneStoreObjectDeclarationWithRefCountBody {
	CompactID oid;
	unsigned int jci : 10;
	unsigned int odcs : 4;
	unsigned int fReserved : 2;
	unsigned int fHasOldReferences : 1;
	unsigned int fHasOsidReferences : 1;
	uint32_t fReserved2 : 30;
} __attribute__((packed)) OneStoreObjectDeclarationWithRefCountBody;

typedef struct _OneStoreObjectDeclarationWithRefCountFNDXData {
	OneStoreObjectDeclarationWithRefCountBody body;
	uint8_t cRef;
} __attribute__((packed)) OneStoreObjectDeclarationWithRefCountFNDXData;

typedef struct _OneStoreObjectDeclarationWithRefCountFNDX {
	OneStoreFileNodeChunkReference ObjectRef;
	OneStoreObjectDeclarationWithRefCountFNDXData data;
} OneStoreObjectDeclarationWithRefCountFNDX;

typedef struct _OneStoreObjectDeclarationWithRefCount2FNDXData {
	OneStoreObjectDeclarationWithRefCountBody body;
	uint32_t cRef;
} OneStoreObjectDeclarationWithRefCount2FNDXData;

typedef struct _OneStoreObjectDeclarationWithRefCount2FNDX {
	OneStoreFileNodeChunkReference ObjectRef;
	OneStoreObjectDeclarationWithRefCount2FNDXData data;
} OneStoreObjectDeclarationWithRefCount2FNDX;

typedef struct _OneStoreObjectDeclaration2Body {
	CompactID oid;
	JCID jcid;
	unsigned int fHasOidReferences : 1;
	unsigned int fHasOsidReferences : 1;
	unsigned int fReserved2 : 6;
} __attribute__((packed)) OneStoreObjectDeclaration2Body;

typedef struct _OneStoreObjectDeclaration2RefCountFNDData {
	OneStoreObjectDeclaration2Body body;
	uint8_t cRef;
} __attribute__((packed)) OneStoreObjectDeclaration2RefCountFNDData;

typedef struct _OneStoreObjectDeclaration2RefCountFND {
	OneStoreFileNodeChunkReference BlobRef;
	OneStoreObjectDeclaration2RefCountFNDData data;
} OneStoreObjectDeclaration2RefCountFND;

typedef struct _OneStoreObjectDeclaration2LargeRefCountFNDData {
	OneStoreObjectDeclaration2Body body;
	uint32_t cRef;
} OneStoreObjectDeclaration2LargeRefCountFNDData;

typedef struct _OneStoreObjectDeclaration2LargeRefCountFND {
	OneStoreFileNodeChunkReference BlobRef;
	OneStoreObjectDeclaration2LargeRefCountFNDData data;
} OneStoreObjectDeclaration2LargeRefCountFND;

typedef struct _OneStoreStringInStorageBuffer {
	uint32_t cch;
	wchar_t *StringData;
} OneStoreStringInStorageBuffer;

typedef struct _OneStoreObjectDeclarationFileData3RefCountFNDHeader {
	CompactID oid;
	JCID jcid;
	uint8_t cRef;
} __attribute__((packed)) OneStoreObjectDeclarationFileData3RefCountFNDHeader;

typedef struct _OneStoreObjectDeclarationFileData3RefCountFND {
	OneStoreObjectDeclarationFileData3RefCountFNDHeader header;
	OneStoreStringInStorageBuffer FileDataReference;
	OneStoreStringInStorageBuffer Extension;
} OneStoreObjectDeclarationFileData3RefCountFND;


typedef struct _OneStoreObjectDeclarationFileData3LargeRefCountFNDHeader {
	CompactID oid;
	JCID jcid;
	uint32_t cRef;
} __attribute__((packed)) OneStoreObjectDeclarationFileData3LargeRefCountFNDHeader;

typedef struct _OneStoreObjectDeclarationFileData3LargeRefCountFND {
	OneStoreObjectDeclarationFileData3LargeRefCountFNDHeader header;
	OneStoreStringInStorageBuffer FileDataReference;
	OneStoreStringInStorageBuffer Extension;
} _OneStoreObjectDeclarationFileData3LargeRefCountFND;

typedef struct _OneStoreReadOnlyObjectDeclaration2RefCountFND {
	OneStoreObjectDeclaration2RefCountFND base;
	uint16_t md5Hash;
} OneStoreReadOnlyObjectDeclaration2RefCountFND;

typedef struct _OneStoreReadOnlyObjectDeclaration2LargeRefCountFND {
	OneStoreObjectDeclaration2LargeRefCountFND base;
	uint16_t md5Hash;
} OneStoreReadOnlyObjectDeclaration2LargeRefCountFND;

typedef struct _OneStoreObjectGroupListReferenceFND {
	OneStoreFileNodeChunkReference ref;
	ExtendedGUID ObjectGroupID;
} OneStoreObjectGroupListReferenceFND;

typedef struct _OneStoreObjectGroupStartFND {
	ExtendedGUID oid;
} OneStoreObjectGroupStartFND;

typedef struct _OneStoreDataSignatureGroupDefinitionFND {
	ExtendedGUID DataSignatureGroup;
} __attribute__((packed)) OneStoreDataSignatureGroupDefinitionFND;

typedef struct _OneStoreFileDataStoreObject {
	GUID guidHeader;
	uint64_t cbLength;
	uint32_t unused;
	uint64_t reserved;
	uint8_t *FileData;
	GUID guidFooter;
} __attribute__((packed)) OneStoreFileDataStoreObject;











/* Other structures */

typedef struct _OneStoreprtFourBytesOfLengthFollowedByData {
	uint32_t cb;
	uint8_t Data[];
} OneStoreprtFourBytesOfLengthFollowedByData;


typedef struct _OneStoreObjectSpaceObjectStreamHeader {
	uint32_t Count : 24;
	uint32_t Reserved : 6;
	unsigned int ExtendedStreamsPresent : 1;
	unsigned int OsidStreamNotPresent : 1;
} OneStoreObjectSpaceObjectStreamHeader;

typedef struct _OneStorePropertyID {
	uint32_t id : 26;
	unsigned int type : 5;
	unsigned int boolValue : 1;
} __attribute__((packed)) OneStorePropertyID;

typedef struct _OneStoreprtArrayOfPropertyValues {
	uint32_t cProperties;
	OneStorePropertyID prid;
	uint8_t Data[];
} OneStoreprtArrayOfPropertyValues;

typedef struct _OneStorePropertySet {
	uint16_t cProperties;
	OneStorePropertyID* rgPrids;
	uint8_t* rgData;
} OneStorePropertySet;

typedef struct _OneStoreObjectSpaceObjectStreamOfSomeIDs {
	OneStoreObjectSpaceObjectStreamHeader header;
	CompactID *body;
} OneStoreObjectSpaceObjectStreamOfOIDs,
  OneStoreObjectSpaceObjectStreamOfOSIDs,
  OneStoreObjectSpaceObjectStreamOfContextIDs;

typedef struct _OneStoreObjectSpaceObjectPropSet {
	OneStoreObjectSpaceObjectStreamOfOIDs *OIDs;
	OneStoreObjectSpaceObjectStreamOfOSIDs *OSIDs;
	OneStoreObjectSpaceObjectStreamOfContextIDs *ContextIDs;
	OneStorePropertySet body;
	uint8_t* padding; // can be ignored 'cause the API i'm writing will not
	// read the struct directly from the file; it'll be read and parsed field by field.
	struct {
		uint64_t osidsOffset;
		uint64_t contextIdsOffset;
		uint64_t propSetOffset;
	} extra;
} OneStoreObjectSpaceObjectPropSet;

typedef struct _OneStore_prtFourBytesOfLengthFollowedByData {
	uint32_t cb;
	uint8_t Data;
} __attribute__((packed)) OneStore_prtFourBytesOfLengthFollowedByData;

typedef struct _OneStore_prtArrayOfPropertyValuesHeader {
	uint32_t cProperties;
	OneStorePropertyID prid;
} OneStore_prtArrayOfPropertyValuesHeader;

typedef struct _OneStore_prtArrayOfPropertyValues {
	OneStore_prtArrayOfPropertyValuesHeader header;
	uint8_t* Data;
} OneStore_prtArrayOfPropertyValues;

typedef union _OneStoreFileNodeData {
	// generic ref field
	OneStoreFileNodeChunkReference ref;
	OneStoreObjectSpaceManifestRootFND objectSpaceManifestRootFND;
	OneStoreObjectSpaceManifestListReferenceFND objectSpaceManifestListReferenceFND;
	OneStoreObjectSpaceManifestListStartFND objectSpaceManifestListStartFND;
	OneStoreRevisionManifestListStartFND revisionManifestListStartFND;
	OneStoreRevisionManifestStart4FND revisionManifestStart4FND;
	OneStoreRevisionManifestStart6FND revisionManifestStart6FND;
	OneStoreRevisionManifestStart7FND revisionManifestStart7FND;
	OneStoreGlobalIdTableStartFNDX globalIdTableStartFNDX;
	OneStoreGlobalIdTableEntryFNDX globalIdTableEntryFNDX;
	OneStoreGlobalIdTableEntry2FNDX globalIdTableEntry2FNDX;
	OneStoreGlobalIdTableEntry3FNDX globalIdTableEntry3FNDX;
	OneStoreObjectRevisionWithRefCountFNDX objectRevisionWithRefCountFNDX;
	OneStoreObjectRevisionWithRef2CountFNDX objectRevisionWithRef2CountFNDX;
	OneStoreRootObjectReference2FNDX rootObjectReference2FNDX;
	OneStoreRootObjectReference3FND rootObjectReference3FND;
	OneStoreRevisionRoleDeclarationFND revisionRoleDeclarationFND;
	OneStoreRevisionRoleAndContextDeclarationFND revisionRoleAndContextDeclarationFND;
	OneStoreObjectInfoDependencyOverridesFND objectInfoDependencyOverridesFND;
	OneStoreFileDataStoreObjectReferenceFND fileDataStoreObjectReferenceFND;
	OneStoreObjectDeclarationWithRefCountFNDX objectDeclarationWithRefCountFNDX;
	OneStoreObjectDeclarationWithRefCount2FNDX objectDeclarationWithRefCount2FNDX;
	OneStoreObjectDeclaration2RefCountFND objectDeclaration2RefCountFND;
	OneStoreObjectDeclaration2LargeRefCountFND objectDeclaration2LargeRefCountFND;
	OneStoreObjectDeclarationFileData3RefCountFND objectDeclarationFileData3RefCountFND;
	OneStoreReadOnlyObjectDeclaration2RefCountFND readOnlyObjectDeclaration2RefCountFND;
	OneStoreReadOnlyObjectDeclaration2LargeRefCountFND readOnlyObjectDeclaration2LargeRefCountFND;
	OneStoreObjectGroupListReferenceFND objectGroupListReferenceFND;
	OneStoreObjectGroupStartFND objectGroupStartFND;
	OneStoreDataSignatureGroupDefinitionFND dataSignatureGroupDefinitionFND;
	OneStoreRevisionManifestListReferenceFND revisionManifestListReferenceFND;
	OneStoreObjectDataEncryptionKeyV2FNDX objectDataEncryptionKeyV2FNDX;
	OneStoreFileDataStoreListReferenceFND fileDataStoreListReferenceFND;
} OneStoreFileNodeData;


typedef struct _OneStoreFileNode {
	OneStoreFileNodeHeader header;
	OneStoreFileNodeData fnd;
	struct {
		size_t fileNodeChunkRefSize;
		uint64_t stp;
		uint8_t refFlag;
		uint8_t otherRefFlag;
	} extra;
} OneStoreFileNode;











/* File Node List Fragment */

#define FILENODELISTFRAGMENT_HEADER_MAGIC_NUMBER 0xA4567AB1F5F7F4C4
#define FILENODELISTFRAGMENT_FOOTER_MAGIC_NUMBER 0x8BC215C38233BA4B

typedef struct {
	// must always be 0xA4567AB1F5F7F4C4
	uint64_t uintMagic;
	uint32_t FileNodeListID;
	uint32_t nFragmentSequence;
} __attribute__((packed)) OneStoreFileNodeListHeader;

// this struct will be filled field by field manually.
typedef struct {
	OneStoreFileNodeListHeader header;
	OneStoreFileNode* rgFileNodes;
	OneStoreFileChunkReference64x32 nextFragment;
	/* must be always 0x8BC215C38233BA4B, which marks the end of fragment */
	uint64_t footer;
	
	struct {
		uint64_t footerMagicNumberStartIndex;
		size_t totalFileNodes;
	} extra;
} OneStoreFileNodeListFragment;









/* Function declarations */

void conote_read_onenote_file_header(OneStoreFileHeaderRaw*, const char*);
void conote_dump_onenote_file_header(OneStoreFileHeaderRaw*, const char*);

void empty();

#ifdef __cplusplus
} // extern C
#endif

#endif // MS_ONESTORE_H