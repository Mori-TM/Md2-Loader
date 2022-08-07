#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

namespace Md2
{
	typedef unsigned char u8;
	typedef char s8;	
	typedef unsigned short u16;
	typedef short s16;
	typedef unsigned int u32;
	typedef int s32;
	typedef float f32;
	typedef double f64;
	typedef struct
	{
		f32 P[3];
	} vec3;

	typedef struct
	{
		f32 S;
		f32 T;
	} TexCoord;

	typedef struct
	{
		s16 S;
		s16 T;
	} STIndex;

	typedef struct
	{
		u8 V[3];
		u8 NormalIndex;
	} FramePoint;

	typedef struct
	{
		f32 Scale[3];
		f32 Translate[3];
		s8 Name[16];
		FramePoint Fp[1];
	} Frame;

	typedef struct
	{
		u16 MeshIndex[3];
		u16 STIndex[3];
	} Mesh;

	typedef struct
	{
		u16 MeshIndex[3];
		u16 STIndex[3];
		u16 NormalIndex[3];
	} MeshExt;

	typedef struct
	{
		s32 Ident;
		s32 Version;
		s32 SkinWidth;
		s32 SkinHeight;
		s32 FrameSize;
		s32 NumSkins;
		s32 NumXYZ;
		s32 NumST;
		s32 NumTris;
		s32 NumGLCmds;
		s32 NumFrames;
		s32 OffsetSkins;
		s32 OffsetST;
		s32 OffsetTris;
		s32 OffsetFrames;
		s32 OffsetGLCmds;
		s32 OffsetEnd;
	} ModelHeader;

	typedef struct
	{
		s32 NumFrames;
		s32 NumPoints;
		s32 NumTriangles;
		s32 NumST;
		s32 FrameSize;
		s32 TexWidth;
		s32 TexHeight;
		s32 CurrentFrame;
		s32 NextFrame;
		f32  Interpol;
		MeshExt* TriIndex;
		TexCoord* ST;
		vec3* PointList;

		vec3* NormalList;
	} ModelData;

	void Normalize(vec3* v)
	{
		register float Length = glFastInverseSqrt(v->P[0] * v->P[0] + v->P[1] * v->P[1] + v->P[2] * v->P[2]);
		v->P[0] *= Length;
		v->P[1] *= Length;
		v->P[2] *= Length;
	}

	vec3 Cross(vec3* a, vec3* b)
	{
		vec3 r;
		r.P[0] = a->P[1] * b->P[2] - a->P[2] * b->P[1];
		r.P[1] = a->P[2] * b->P[0] - a->P[0] * b->P[2];
		r.P[2] = a->P[0] * b->P[1] - a->P[1] * b->P[0];
		return r;
	}

	vec3 Sub(vec3* a, vec3* b)
	{
		vec3 r;
		r.P[0] = a->P[0] - b->P[0];
		r.P[1] = a->P[1] - b->P[1];
		r.P[2] = a->P[2] - b->P[2];
		return r;
	}

	void GenerateNormals(u32 VertexCount, vec3* Vertices, u32 IndexCount, MeshExt* Indices, vec3* Normals, u32 j)
	{
		vec3* PointList = &Vertices[VertexCount * j];
		vec3* NormalList = &Normals[VertexCount * j];

		for (u32 i = 0; i < VertexCount; i++)
		{
			NormalList[i].P[0] = 0.0;
			NormalList[i].P[1] = 0.0;
			NormalList[i].P[2] = 0.0;
		}
		for (u32 i = 0; i < IndexCount; i++)
		{
			vec3* V0 = &PointList[Indices[i].MeshIndex[0]];
			vec3* V1 = &PointList[Indices[i].MeshIndex[2]];
			vec3* V2 = &PointList[Indices[i].MeshIndex[1]];

			vec3 A = Sub(V1, V0);
			vec3 B = Sub(V2, V0);
			vec3 Normal = Cross(&A, &B);
			Normalize(&Normal);

			NormalList[Indices[i].MeshIndex[0]].P[0] += Normal.P[0];
			NormalList[Indices[i].MeshIndex[0]].P[1] += Normal.P[1];
			NormalList[Indices[i].MeshIndex[0]].P[2] += Normal.P[2];
															  
			NormalList[Indices[i].MeshIndex[2]].P[0] += Normal.P[0];
			NormalList[Indices[i].MeshIndex[2]].P[1] += Normal.P[1];
			NormalList[Indices[i].MeshIndex[2]].P[2] += Normal.P[2];
															  
			NormalList[Indices[i].MeshIndex[1]].P[0] += Normal.P[0];
			NormalList[Indices[i].MeshIndex[1]].P[1] += Normal.P[1];
			NormalList[Indices[i].MeshIndex[1]].P[2] += Normal.P[2];
		}

		for (u32 i = 0; i < VertexCount; i++)
			Normalize(&NormalList[i]);
	}

	s8 Load(const s8* FileName, s32 TexWidth, s32 TexHeight, ModelData* Model)
	{		
		ModelHeader* Header;

		STIndex* STIndices;
		Frame* FrameData;
		vec3* PointList;
		MeshExt* TriIndex;
		Mesh* BufIndex;
		s32 i;
		s32 j;

		FILE* File = fopen(FileName, "rb");
		if (!File)
			return 0;

		fseek(File, 0, SEEK_END);
		s32 FileLength = ftell(File);
		fseek(File, 0, SEEK_SET);

		s8* Buffer = (s8*)malloc(FileLength + 1);
		fread(Buffer, 1, FileLength, File);
		fclose(File);

		Header = (ModelHeader*)Buffer;
		printf("d: %d\n", Header->NumXYZ);
		Model->PointList = (vec3*)malloc(Header->NumXYZ * Header->NumFrames * sizeof(vec3));
		//-----//
		Model->NormalList = (vec3*)malloc(Header->NumXYZ * Header->NumFrames * sizeof(vec3));
		//-----//
		Model->NumPoints = Header->NumXYZ;
		Model->NumFrames = Header->NumFrames;
		Model->FrameSize = Header->FrameSize;

		for (j = 0; j < Header->NumFrames; j++)
		{
			FrameData = (Frame*)&Buffer[Header->OffsetFrames + Header->FrameSize * j];

			PointList = (vec3*)&Model->PointList[Header->NumXYZ * j];
			for (i = 0; i < Header->NumXYZ; i++)
			{
				PointList[i].P[0] = FrameData->Scale[0] * FrameData->Fp[i].V[0] + FrameData->Translate[0];
				PointList[i].P[1] = FrameData->Scale[1] * FrameData->Fp[i].V[1] + FrameData->Translate[1];
				PointList[i].P[2] = FrameData->Scale[2] * FrameData->Fp[i].V[2] + FrameData->Translate[2];
			}
		}

		Model->ST = (TexCoord*)malloc(Header->NumST * sizeof(TexCoord));
		Model->NumST = Header->NumST;

		STIndices = (STIndex*)&Buffer[Header->OffsetST];
		for (i = 0; i < Header->NumST; i++)
		{
			Model->ST[i].S = (f32)STIndices[i].S / (f32)TexWidth;
			Model->ST[i].T = (f32)STIndices[i].T / (f32)TexHeight;
		}

		TriIndex = (MeshExt*)malloc(Header->NumTris * sizeof(MeshExt));

		Model->NumTriangles = Header->NumTris;
		Model->TriIndex = TriIndex;

		BufIndex = (Mesh*)&Buffer[Header->OffsetTris];

		//why this loop it doesnt do anything
		for (j = 0; j < Model->NumFrames; j++)
		{
			for (i = 0; i < Header->NumTris; i++)
			{
				TriIndex[i].MeshIndex[0] = BufIndex[i].MeshIndex[0];
				TriIndex[i].MeshIndex[1] = BufIndex[i].MeshIndex[1];
				TriIndex[i].MeshIndex[2] = BufIndex[i].MeshIndex[2];

				TriIndex[i].STIndex[0] = BufIndex[i].STIndex[0];
				TriIndex[i].STIndex[1] = BufIndex[i].STIndex[1];
				TriIndex[i].STIndex[2] = BufIndex[i].STIndex[2];

				TriIndex[i].NormalIndex[0] = BufIndex[i].MeshIndex[0];
				TriIndex[i].NormalIndex[1] = BufIndex[i].MeshIndex[1];
				TriIndex[i].NormalIndex[2] = BufIndex[i].MeshIndex[2];
			}

			GenerateNormals(Header->NumXYZ, Model->PointList, Header->NumTris, TriIndex, Model->NormalList, j);
		}

		free(Buffer);

		Model->CurrentFrame = 0;
		Model->NextFrame = 1;
		Model->Interpol = 0.0;

		return 1;
	}

	void Free(ModelData* Model)
	{
		free(Model->TriIndex);
		free(Model->ST);
		free(Model->PointList);
	}

	class Proccess
	{
	public:
		ModelData* Model;
		vec3 Vertex[3];
		TexCoord TexCoord[3];
		vec3 Normal[3];

	private:
		f32 X0, Y0, Z0;
		f32 X1, Y1, Z1;
		vec3* PointList;
		vec3* NextPointList;
		vec3* NormalList;

	public:
		void Start(s32 StartFrame, s32 EndFrame)
		{
			if (StartFrame > Model->CurrentFrame)
				Model->CurrentFrame = StartFrame;

		//	if ((StartFrame > Model->CurrentFrame))


			if (Model->Interpol >= 1.0)
			{
				Model->Interpol = 0.0;
				Model->CurrentFrame++;

				if (Model->CurrentFrame >= EndFrame)
					Model->CurrentFrame = StartFrame;

				Model->NextFrame = Model->CurrentFrame + 1;

				if (Model->NextFrame >= EndFrame)
					Model->NextFrame = StartFrame;
			}

			PointList = &Model->PointList[Model->NumPoints * Model->CurrentFrame];
			NextPointList = &Model->PointList[Model->NumPoints * Model->NextFrame];
			NormalList = &Model->NormalList[Model->NumPoints * Model->CurrentFrame];
		}

		void End(f32 Percent)
		{
			Model->Interpol += Percent;
		}

		void Update(s32 i)
		{
			X0 = PointList[Model->TriIndex[i].MeshIndex[0]].P[0];
			Y0 = PointList[Model->TriIndex[i].MeshIndex[0]].P[1];
			Z0 = PointList[Model->TriIndex[i].MeshIndex[0]].P[2];

			X1 = NextPointList[Model->TriIndex[i].MeshIndex[0]].P[0];
			Y1 = NextPointList[Model->TriIndex[i].MeshIndex[0]].P[1];
			Z1 = NextPointList[Model->TriIndex[i].MeshIndex[0]].P[2];

			Vertex[0].P[0] = X0 + Model->Interpol * (X1 - X0);
			Vertex[0].P[1] = Y0 + Model->Interpol * (Y1 - Y0);
			Vertex[0].P[2] = Z0 + Model->Interpol * (Z1 - Z0);

			//---------//
			X0 = PointList[Model->TriIndex[i].MeshIndex[2]].P[0];
			Y0 = PointList[Model->TriIndex[i].MeshIndex[2]].P[1];
			Z0 = PointList[Model->TriIndex[i].MeshIndex[2]].P[2];

			X1 = NextPointList[Model->TriIndex[i].MeshIndex[2]].P[0];
			Y1 = NextPointList[Model->TriIndex[i].MeshIndex[2]].P[1];
			Z1 = NextPointList[Model->TriIndex[i].MeshIndex[2]].P[2];

			Vertex[2].P[0] = X0 + Model->Interpol * (X1 - X0);
			Vertex[2].P[1] = Y0 + Model->Interpol * (Y1 - Y0);
			Vertex[2].P[2] = Z0 + Model->Interpol * (Z1 - Z0);

			//---------//
			X0 = PointList[Model->TriIndex[i].MeshIndex[1]].P[0];
			Y0 = PointList[Model->TriIndex[i].MeshIndex[1]].P[1];
			Z0 = PointList[Model->TriIndex[i].MeshIndex[1]].P[2];

			X1 = NextPointList[Model->TriIndex[i].MeshIndex[1]].P[0];
			Y1 = NextPointList[Model->TriIndex[i].MeshIndex[1]].P[1];
			Z1 = NextPointList[Model->TriIndex[i].MeshIndex[1]].P[2];

			Vertex[1].P[0] = X0 + Model->Interpol * (X1 - X0);
			Vertex[1].P[1] = Y0 + Model->Interpol * (Y1 - Y0);
			Vertex[1].P[2] = Z0 + Model->Interpol * (Z1 - Z0);

			TexCoord[0] = Model->ST[Model->TriIndex[i].STIndex[0]];
			TexCoord[2] = Model->ST[Model->TriIndex[i].STIndex[2]];
			TexCoord[1] = Model->ST[Model->TriIndex[i].STIndex[1]];
			
			
			Normal[0] = NormalList[Model->TriIndex[i].NormalIndex[0]];
			Normal[2] = NormalList[Model->TriIndex[i].NormalIndex[2]];
			Normal[1] = NormalList[Model->TriIndex[i].NormalIndex[1]];
		}
	};
	
}
