#ifndef EASY_STUDIO_HEADER_H
#define EASY_STUDIO_HEADER_H

#pragma once

inline static constexpr uint32_t kMaxMDLFileBuffer = 16 * 1024 * 1024;

inline static constexpr uint32_t kMaxModelHeaderName = 64;
inline static constexpr uint32_t kMaxModelTextureName = 64;
inline static constexpr uint32_t kMaxBoneName = 32;
inline static constexpr uint32_t kMaxSequenceLabel = 32;
inline static constexpr uint32_t kMaxSequenceGroupLabel = 32;
inline static constexpr uint32_t kMaxSequenceGroupFilename = 64;
inline static constexpr uint32_t kMaxBodypartName = 64;
inline static constexpr uint32_t kMaxAttachmentName = 32;
inline static constexpr uint32_t kMaxSubModelName = 64;

inline static constexpr uint32_t kMaxSequenceBlendCount = 2;

// little-endian "IDST" & "IDSQ"
inline static constexpr int32_t kIDStudioHeader = ('T' << 24 ) + ('S' << 16) + ('D' << 8) + 'I';
inline static constexpr int32_t kIDStudioSeqHeader = ('Q' << 24) + ('S' << 16) + ('D' << 8) + 'I';

inline static constexpr uint32_t kStudioVersion = 10;

inline static constexpr double kPI = 3.14159265358979323846;

#define ALIGN( a ) a = (uint8_t *)((int32_t)((uint8_t *)a + 3) & ~ 3)

// motion flags
#define STUDIO_X		0x0001
#define STUDIO_Y		0x0002	
#define STUDIO_Z		0x0004
#define STUDIO_XR		0x0008
#define STUDIO_YR		0x0010
#define STUDIO_ZR		0x0020
#define STUDIO_LX		0x0040
#define STUDIO_LY		0x0080
#define STUDIO_LZ		0x0100
#define STUDIO_AX		0x0200
#define STUDIO_AY		0x0400
#define STUDIO_AZ		0x0800
#define STUDIO_AXR		0x1000
#define STUDIO_AYR		0x2000
#define STUDIO_AZR		0x4000

#define STUDIO_TYPES	0x7FFF
#define STUDIO_RLOOP	0x8000	// controller that wraps shortest distance

enum Studio10Limits
{
	MAX_TRIANGLES				= 20000,	// TODO: tune this
	MAX_VERTS					= 2048,		// TODO: tune this
	MAX_SEQUENCES				= 2048,		// total animation sequences -- KSH incremented
	MAX_SKINS					= 100,		// total textures
	MAX_SRCBONES					= 512,		// bones allowed at source movement
	MAX_BONES					= 128,		// total bones actually used
	MAX_MODELS					= 32,		// sub-models per model
	MAX_BODYPARTS				= 32,
	MAX_GROUPS					= 16,
	MAX_ANIMATIONS				= 2048,		
	MAX_MESHES					= 256,
	MAX_EVENTS					= 1024,
	MAX_PIVOTS					= 256,
	MAX_CONTROLLERS				= 8,

	MAX_EVENT_OPTIONS_LENGTH 	= 64,

	MAX_NUM_COORDINATE_AXES		= 6,

	MAX_ATTACH_NUM_VECTORS		= 3
};

// skin info
class StudioTexture
{
public:
	char		name[kMaxModelTextureName];
	int32_t		flags;
	int32_t		width;
	int32_t		height;
	int32_t		index;
};

// bones
class StudioBone
{
public:
	char		name[kMaxBoneName];	// bone name for symbolic links
	int32_t		parent;		// parent bone
	int32_t		flags;		// ??
	int32_t		bonecontroller[MAX_NUM_COORDINATE_AXES];	// bone controller index, -1 == none
	float		value[MAX_NUM_COORDINATE_AXES];	// default DoF values
	float		scale[MAX_NUM_COORDINATE_AXES];   // scale for delta DoF values
};

// bone controllers
class StudioBoneController
{
public:
	int32_t		bone;	// -1 == 0
	int32_t		type;	// X, Y, Z, XR, YR, ZR, M
	float		start;
	float		end;
	int32_t		rest;	// byte index value at rest
	int32_t		index;	// 0-3 user set controller, 4 mouth
};

// intersection boxes
class StudioBBox
{
public:
	int32_t		bone;
	int32_t		group;	// intersection group
	Vector		bbmin;	// bounding box
	Vector		bbmax;
};

// sequence descriptions
class StudioSequenceDescription
{
public:
	char		label[kMaxSequenceLabel];	// sequence label

	float		framerate;	// frames per second	
	int32_t		flags;		// looping/non-looping flags

	int32_t		activity;
	int32_t		actweight;

	class Event
	{
	public:
		int32_t frame;
		int32_t event;
		int32_t type;
		char 	options[MAX_EVENT_OPTIONS_LENGTH];
	};

	int32_t		numevents;
	int32_t		eventoffset;
	Event* events(void *pHeader) const
	{
		return reinterpret_cast<Event*>((uint8_t*)pHeader + eventoffset);
	}

	int32_t		numframes;	// number of frames per sequence

	class Pivot
	{
	public:
		Vector org;
		int32_t start, end;
	};

	int32_t		numpivots;	// number of foot pivots
	int32_t		pivotoffset;
	Pivot* pivots(void *pHeader) const
	{
		return reinterpret_cast<Pivot*>((uint8_t*)pHeader + pivotoffset);
	}

	int32_t		motiontype;
	int32_t		motionbone;
	Vector		linearmovement;
	int32_t		automoveposindex;
	int32_t		automoveangleindex;

	Vector		bbmin;			// per sequence bounding box
	Vector		bbmax;

	class Animation
	{
	public:
		static inline constexpr uint32_t kNumValues = 6;
		uint16_t offset[kNumValues];
		class Value
		{
		public:

			struct
			{
				uint8_t valid;
				uint8_t total;
			} num;
			uint16_t value;
		};
		Value* values(Animation* pAnimation, int32_t idx) const
		{
			return reinterpret_cast<Value*>((uint8_t*)pAnimation + offset[idx]);
		}
	};

	int32_t		numblends;		// TODO: rename struct
	int32_t		animoffset;		// mstudioanim_t pointer relative to start of sequence group data
								// [blend][bone][X, Y, Z, XR, YR, ZR]
	Animation* animations(void *pHeader) const
	{
		return reinterpret_cast<Animation*>((uint8_t*)pHeader + animoffset);
	}

	int32_t		blendtype[kMaxSequenceBlendCount];	// X, Y, Z, XR, YR, ZR
	float		blendstart[kMaxSequenceBlendCount];	// starting value
	float		blendend[kMaxSequenceBlendCount];	// ending value
	int32_t		blendparent;

	int32_t		seqgroup;		// sequence group for demand loading

	int32_t		entrynode;		// transition node at entry
	int32_t		exitnode;		// transition node at exit
	int32_t		nodeflags;		// transition rules

	int32_t		nextseq;		// auto advancing sequences
};

// demand loaded sequence groups
class StudioSequenceGroup
{
public:
	char	label[kMaxSequenceGroupLabel];	 // textual name
	char	name[kMaxSequenceGroupFilename]; // file name
	int32_t	unused1; // was "cache"  - index pointer
	int32_t	unused2; // was "data" -  hack for group 0
};

// replaceable textures
class StudioSkinRef
{
public:
	uint16_t* skinref;
};

// body part index
class StudioBodyPart
{
public:
	char		name[kMaxBodypartName];

	// Sub model
	class StudioSubModel
	{
	public:
		char		name[kMaxSubModelName];

		int32_t		type;

		float		boundingradius;

		class Mesh
		{
		public:
			class Triangle
			{
			public:
				uint16_t* tricmd;
			};

			int32_t numtris;
			int32_t trioffset;
			Triangle* triangles(void* pHeader) const
			{
				return reinterpret_cast<Triangle*>((uint8_t*)pHeader + trioffset);
			}

			int32_t skinref;

			class Normal
			{
			public:
				Vector normal;
			};

			int32_t numnorms;	// per mesh normals
			int32_t normoffset;	// normal vec3_t
			Normal* normals(void* pHeader) const
			{
				return reinterpret_cast<Normal*>((uint8_t*)pHeader + normoffset);
			}
		};

		int32_t		nummesh;
		int32_t		meshoffset;
		Mesh* meshes(void* pHeader) const
		{
			return reinterpret_cast<Mesh*>((uint8_t*)pHeader + meshoffset);
		}

		class Vertex
		{
		public:
			Vector vertex;
		};

		class VertexInfo
		{
		public:
			uint8_t bone;
		};

		int32_t		numverts;		// number of unique vertices
		int32_t		vertinfooffset;	// vertex bone info
		int32_t		vertoffset;		// vertex vec3_t
		Vertex* vertexes(void* pHeader) const
		{
			return reinterpret_cast<Vertex*>((uint8_t*)pHeader + vertoffset);
		}
		VertexInfo* vertexesinfo(void* pHeader) const
		{
			return reinterpret_cast<VertexInfo*>((uint8_t*)pHeader + vertinfooffset);
		}

		class Normal
		{
		public:
			Vector normal;
		};

		class NormalInfo
		{
		public:
			uint8_t bone;
		};

		int32_t		numnorms;		// number of unique surface normals
		int32_t		norminfooffset;	// normal bone info
		int32_t		normoffset;		// normal vec3_t
		Normal* normals(void* pHeader) const
		{
			return reinterpret_cast<Normal*>((uint8_t*)pHeader + normoffset);
		}
		NormalInfo* normalsinfo(void* pHeader) const
		{
			return reinterpret_cast<NormalInfo*>((uint8_t*)pHeader + norminfooffset);
		}

		// deformation groups
		// TODO: Unused in goldsrc?
		int32_t		numgroups;
		int32_t		groupoffset;
	};

	int32_t		nummodels;
	int32_t		base;
	int32_t		modelindex; // index into models array
	StudioSubModel* submodels(void* pHeader) const
	{
		return reinterpret_cast<StudioSubModel*>((uint8_t*)pHeader + modelindex);
	}
};

// attachment
class StudioAttachment
{
public:
	char name[kMaxAttachmentName]; // Name of this attachment. Unused in GoldSource.
	
	int32_t type; // Type of this attachment. Unused in GoldSource;
	int32_t bone; // Index of the bone this is attached to.

	Vector org; // Offset from bone origin.
	Vector vectors[MAX_ATTACH_NUM_VECTORS]; // Directional vectors? Unused in GoldSource.
};

// Transitions
class StudioTransition
{
public:
	uint8_t* transition;
};

// header for demand loaded sequence group data
class StudioHeader
{
public:
	int32_t		id;
	int32_t		version;

	char		name[kMaxModelHeaderName];
	int32_t		length;

	Vector		eyepos;	// ideal eye position
	Vector		min;	// ideal movement hull size
	Vector		max;

	Vector		bbmin; // clipping bounding box
	Vector		bbmax;		

	int32_t		flags;

	// bones
	int32_t		numbones;
	int32_t		boneoffset;
	inline auto bones() const
	{
		return reinterpret_cast<StudioBone*>((uint8_t*)this + boneoffset);
	}

	// bone controllers
	int32_t		numbonecontrollers;
	int32_t		bonecontrolleroffset;
	inline auto bonecontrollers() const
	{
		return reinterpret_cast<StudioBoneController*>((uint8_t*)this + bonecontrolleroffset);
	}

	// complex bounding boxes
	int32_t		numhitboxes;
	int32_t		hitboxoffset;
	inline auto hitboxes() const
	{
		return reinterpret_cast<StudioBBox*>((uint8_t*)this + hitboxoffset);
	}
	
	// animation sequences
	int32_t		numseq;
	int32_t		seqoffset;
	inline auto sequences() const
	{
		return reinterpret_cast<StudioSequenceDescription*>((uint8_t*)this + seqoffset);
	}

	// demand loaded sequences
	int32_t		numseqgroups;
	int32_t		seqgroupoffset;
	inline auto sequencegroups() const
	{
		return reinterpret_cast<StudioSequenceGroup*>((uint8_t*)this + seqgroupoffset);
	}

	// raw textures
	int32_t		numtextures; 
	int32_t		textureoffset;
	int32_t		texturedataoffset;
	inline auto textures() const
	{
		return reinterpret_cast<StudioTexture*>((uint8_t*)this + textureoffset);
	}
	StudioTexture* texture_by_name(const char* name)
	{
		const auto tex = textures();
		for (int32_t i = 0; i < numtextures; i++)
		{
			if (!&tex[i])
				continue;

			if (stricmp(tex[i].name, name))
				continue;

			return &tex[i];
		}

		return nullptr;
	}

	// replaceable textures
	int32_t		numskinref;
	int32_t		numskinfamilies;
	int32_t		skinoffset;
	inline auto skinrefs() const
	{
		return reinterpret_cast<StudioSkinRef*>((uint8_t*)this + skinoffset);
	}

	int32_t		numbodyparts;
	int32_t		bodypartoffset;
	inline auto bodyparts() const
	{
		return reinterpret_cast<StudioBodyPart*>((uint8_t*)this + bodypartoffset);
	}

	int32_t		numattachments; // queryable attachable points
	int32_t		attachmentoffset;
	inline auto attachments() const
	{
		return reinterpret_cast<StudioAttachment*>((uint8_t*)this + attachmentoffset);
	}

	// TODO: These below unused?
	int32_t		numsoundtables;
	int32_t		soundoffset;
	void* sounds() const = delete;

	int32_t		numsoundgroups;
	int32_t		soundgroupoffset;
	void* soundgroups() const = delete;

	int32_t		numtransitions; // animation node to animation node transition graph
	int32_t		transitionoffset;
	inline auto transitions() const
	{
		return reinterpret_cast<StudioTransition*>((uint8_t*)this + transitionoffset);
	}

	const char* id_string()
	{
		switch (id)
		{
			case kIDStudioHeader: 	 return "IDST";
			case kIDStudioSeqHeader: return "IDSQ";
			default: 			  	 return "unknown";
		}
	}
};

#endif