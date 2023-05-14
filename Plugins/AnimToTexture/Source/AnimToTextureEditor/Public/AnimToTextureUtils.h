// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"

namespace AnimToTexture_Private
{
	// Forward declaration
	template <uint16 NumInfluences> struct TVertexSkinWeight;

	struct FVector4u16
	{
		uint16 X;
		uint16 Y;
		uint16 Z;
		uint16 W;
	};

	struct FLowPrecision
	{
		using ColorType = FColor;
		static constexpr EPixelFormat PixelFormat = EPixelFormat::PF_B8G8R8A8;
		static constexpr ETextureSourceFormat TextureSourceFormat = ETextureSourceFormat::TSF_BGRA8;
		static constexpr TextureCompressionSettings CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		static constexpr ColorType DefaultColor = FColor(0, 0, 0);
	};

	struct FHighPrecision
	{
		using ColorType = FVector4u16;
		static constexpr EPixelFormat PixelFormat = EPixelFormat::PF_R16G16B16A16_UNORM;
		static constexpr ETextureSourceFormat TextureSourceFormat = ETextureSourceFormat::TSF_RGBA16;
		static constexpr TextureCompressionSettings CompressionSettings = TextureCompressionSettings::TC_HDR;
		static constexpr ColorType DefaultColor = { 0, 0, 0, 0 };
	};


	/** Writes list of vectors into texture
	*   Note: They must be pre-normalized. */
	template<class V, class TextureSettings>
	bool WriteVectorsToTexture(const TArray<V>& Vectors,
		const int32 NumFrames, const int32 RowsPerFrame,
		const int32 Height, const int32 Width, 
		UTexture2D* Texture);

	/* Writes list of skinweights into texture.
	*  The SkinWeights data is already in uint8 & uint16 format, no need for normalizing it.
	*/
	bool WriteSkinWeightsToTexture(const TArray<TVertexSkinWeight<4>>& SkinWeights,
		const int32 RowsPerFrame,
		const int32 Height, const int32 Width,
		UTexture2D* Texture);

	/* Helper utility for writing 8 or 16 bits textures */
	template<class TextureSettings>
	bool WriteToTexture(UTexture2D* Texture, const uint32 Height, const uint32 Width, const TArray<typename TextureSettings::ColorType>& Data);

	template<class V /* FVector / FVector4 / FIntVector */, class C /* FColor / FLinearColor */>
	void VectorToColor(const V& Vector, C& Color);
	
	//template<class C /* FColor / FLinearColor */, class T /* uint8 / float */>
	//void ColorArrayToData(const TArray<C>& Colors, TArray<T>& Data);
}

// ----------------------------------------------------------------------------

template<>
FORCEINLINE void AnimToTexture_Private::VectorToColor(const FVector3f& Vector, FColor& Color)
{
	const float ClampedX = FMath::Clamp(Vector.X, 0.0f, 1.0f);
	const float ClampedY = FMath::Clamp(Vector.Y, 0.0f, 1.0f);
	const float ClampedZ = FMath::Clamp(Vector.Z, 0.0f, 1.0f);

	Color.R = (uint8)FMath::RoundToInt(ClampedX * 255.0f);
	Color.G = (uint8)FMath::RoundToInt(ClampedY * 255.0f);
	Color.B = (uint8)FMath::RoundToInt(ClampedZ * 255.0f);
	Color.A = 255;
}

template<>
FORCEINLINE void AnimToTexture_Private::VectorToColor(const FVector3d& Vector, FColor& Color)
{
	VectorToColor((FVector3f)Vector, Color);
}


template<>
FORCEINLINE void AnimToTexture_Private::VectorToColor(const FVector4& Vector, FColor& Color)
{
	const float ClampedX = FMath::Clamp(Vector.X, 0.0f, 1.0f);
	const float ClampedY = FMath::Clamp(Vector.Y, 0.0f, 1.0f);
	const float ClampedZ = FMath::Clamp(Vector.Z, 0.0f, 1.0f);
	const float ClampedW = FMath::Clamp(Vector.W, 0.0f, 1.0f);
	
	Color.R = (uint8)FMath::RoundToInt(ClampedX * 255.0f);
	Color.G = (uint8)FMath::RoundToInt(ClampedY * 255.0f);
	Color.B = (uint8)FMath::RoundToInt(ClampedZ * 255.0f);
	Color.A = (uint8)FMath::RoundToInt(ClampedW * 255.0f);
}

template<>
FORCEINLINE void AnimToTexture_Private::VectorToColor(const FIntVector& Vector, FColor& Color)
{
	const int32 ClampedX = FMath::Clamp(Vector.X, 0, 1);
	const int32 ClampedY = FMath::Clamp(Vector.Y, 0, 1);
	const int32 ClampedZ = FMath::Clamp(Vector.Z, 0, 1);

	Color.R = (uint8)(ClampedX * 255);
	Color.G = (uint8)(ClampedY * 255);
	Color.B = (uint8)(ClampedZ * 255);
	Color.A = 255;
}

template<>
FORCEINLINE void AnimToTexture_Private::VectorToColor(const FIntVector4& Vector, FColor& Color)
{
	const int32 ClampedX = FMath::Clamp(Vector.X, 0, 1);
	const int32 ClampedY = FMath::Clamp(Vector.Y, 0, 1);
	const int32 ClampedZ = FMath::Clamp(Vector.Z, 0, 1);
	const int32 ClampedW = FMath::Clamp(Vector.W, 0, 1);

	Color.R = (uint8)(ClampedX * 255);
	Color.G = (uint8)(ClampedY * 255);
	Color.B = (uint8)(ClampedZ * 255);
	Color.A = (uint8)(ClampedW * 255);
}

template<>
FORCEINLINE void AnimToTexture_Private::VectorToColor(const FVector3f& Vector, FLinearColor& Color)
{
	Color.R = Vector.X;
	Color.G = Vector.Y;
	Color.B = Vector.Z;
	Color.A = 1.0f;
}

template<>
FORCEINLINE void AnimToTexture_Private::VectorToColor(const FVector4& Vector, FLinearColor& Color)
{
	Color.R = Vector.X;
	Color.G = Vector.Y;
	Color.B = Vector.Z;
	Color.A = Vector.W;
}

template<>
FORCEINLINE void AnimToTexture_Private::VectorToColor(const FVector3f& Vector, FVector4u16& Color)
{
	Color.X = FMath::RoundToInt(FMath::Clamp(Vector.X, 0.0f, 1.0f) * MAX_uint16);
	Color.Y = FMath::RoundToInt(FMath::Clamp(Vector.Y, 0.0f, 1.0f) * MAX_uint16);
	Color.Z = FMath::RoundToInt(FMath::Clamp(Vector.Z, 0.0f, 1.0f) * MAX_uint16);
	Color.W = MAX_uint16;
}

template<>
FORCEINLINE void AnimToTexture_Private::VectorToColor(const FVector4& Vector, FVector4u16& Color)
{
	Color.X = FMath::RoundToInt(FMath::Clamp(Vector.X, 0.0f, 1.0f) * MAX_uint16);
	Color.Y = FMath::RoundToInt(FMath::Clamp(Vector.Y, 0.0f, 1.0f) * MAX_uint16);
	Color.Z = FMath::RoundToInt(FMath::Clamp(Vector.Z, 0.0f, 1.0f) * MAX_uint16);
	Color.W = FMath::RoundToInt(FMath::Clamp(Vector.W, 0.0f, 1.0f) * MAX_uint16);
}

/*
template<>
FORCEINLINE void AnimToTexture_Private::ColorArrayToData(const TArray<FColor>& Colors, TArray<uint8>& Data)
{
	// Resize Array
	Data.Init(0, Colors.Num() * 4);

	for (int32 Index = 0; Index < Colors.Num(); ++Index)
	{
		// NOTE: format is BGRA (swaping R <-> B)
		Data[Index * 4 + 0] = Colors[Index].B; // B
		Data[Index * 4 + 1] = Colors[Index].G; // G
		Data[Index * 4 + 2] = Colors[Index].R; // R
		Data[Index * 4 + 3] = Colors[Index].A; // A
	}
}

template<>
FORCEINLINE void AnimToTexture_Private::ColorArrayToData(const TArray<FLinearColor>& Colors, TArray<float>& Data)
{
	// Resize Array
	Data.Init(0.0f, Colors.Num() * 4);

	for (int32 Index = 0; Index < Colors.Num(); ++Index)
	{
		// NOTE: format is RGBA
		Data[Index * 4 + 0] = Colors[Index].R; // R
		Data[Index * 4 + 1] = Colors[Index].G; // G
		Data[Index * 4 + 2] = Colors[Index].B; // B
		Data[Index * 4 + 3] = Colors[Index].A; // A
	}
}
*/


template<class V, class TextureSettings>
FORCEINLINE bool AnimToTexture_Private::WriteVectorsToTexture(const TArray<V>& Vectors,
	const int32 NumFrames, const int32 RowsPerFrame,
	const int32 Height, const int32 Width, UTexture2D* Texture)
{
	if (!Texture || !NumFrames)
	{
		return false;
	}

	// NumElements Per-Frame
	const int32 NumElements = Vectors.Num() / NumFrames;

	// Allocate PixelData.
	TArray<typename TextureSettings::ColorType> Pixels;
	Pixels.Init(TextureSettings::DefaultColor, Height * Width);

	// Fillout Frame Data
	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		const int32 BlockStart = RowsPerFrame * Width * Frame;
		
		// Set Data.
		for (int32 Index = 0; Index < NumElements; ++Index)
		{
			const V& Vector = Vectors[NumElements * Frame + Index];
			typename TextureSettings::ColorType& Pixel = Pixels[BlockStart + Index];

			VectorToColor<V, typename TextureSettings::ColorType>(Vector, Pixel);
		}
	}

	// Write to Texture
	return WriteToTexture<TextureSettings>(Texture, Height, Width, Pixels);
}

template<class TextureSettings>
FORCEINLINE bool AnimToTexture_Private::WriteToTexture(
	UTexture2D* Texture, 
	const uint32 Height, 
	const uint32 Width, 
	const TArray<typename TextureSettings::ColorType>& Pixels)
{
	if (!Texture)
	{
		return false;
	}

	// ------------------------------------------------------------------------
	// Get Texture Platform
	FTexturePlatformData* PlatformData = Texture->GetPlatformData();
	if (!PlatformData)
	{	
		PlatformData = new FTexturePlatformData();
		Texture->SetPlatformData(PlatformData);
	}
	PlatformData->SizeX = Width;
	PlatformData->SizeY = Height;
	PlatformData->SetNumSlices(1);
	PlatformData->PixelFormat = TextureSettings::PixelFormat;
	
	// ------------------------------------------------------------------------
	// Get First MipMap
	//
	FTexture2DMipMap* Mip;
	if (PlatformData->Mips.IsEmpty())
	{
		Mip = new FTexture2DMipMap();
		PlatformData->Mips.Add(Mip);
	}
	else
	{
		Mip = &PlatformData->Mips[0];
	}
	Mip->SizeX = Width;
	Mip->SizeY = Height;
	
	// ------------------------------------------------------------------------
	// Lock the Mipmap data so it can be modified
	Mip->BulkData.Lock(LOCK_READ_WRITE);

	// Reallocate MipMap
	uint8* TextureData = (uint8*)Mip->BulkData.Realloc(Width * Height * sizeof(typename TextureSettings::ColorType));

	// Copy the pixel data into the Texture data	
	const uint8* PixelsData = (uint8*)Pixels.GetData();
	FMemory::Memcpy(TextureData, PixelsData, Width * Height * sizeof(typename TextureSettings::ColorType));

	// Unlock data
	Mip->BulkData.Unlock();

	// Initialize a new texture
	Texture->Source.Init(Width, Height, 1, 1, TextureSettings::TextureSourceFormat, PixelsData);
	
	// Set parameters
	Texture->SRGB = 0;
	Texture->Filter = TextureFilter::TF_Nearest;
	Texture->CompressionSettings = TextureSettings::CompressionSettings;
	Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;

	// Update and Mark to Save.
	Texture->UpdateResource();
	Texture->MarkPackageDirty();

	return true;
}


