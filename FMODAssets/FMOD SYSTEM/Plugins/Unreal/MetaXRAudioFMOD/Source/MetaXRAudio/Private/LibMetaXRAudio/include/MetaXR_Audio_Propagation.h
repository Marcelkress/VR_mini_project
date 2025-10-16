// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

/********************************************************************************/ /**
 \file      MetaXR_Audio_Propagation.h
 \brief     OVR Audio SDK public header file
 ************************************************************************************/

#ifndef OVR_Audio_Propagation_h
#define OVR_Audio_Propagation_h

#include "MetaXR_Audio_Errors.h"

#include <stddef.h>
#include <stdint.h>

#ifndef OVRA_EXPORT
#if defined(__GNUC__)
#define OVRA_EXPORT __attribute__((visibility("default")))
#elif defined(_MSC_VER)
#define OVRA_EXPORT __declspec(dllexport)
#else
#error Unsupported compiler
#endif
#endif

#ifndef OVR_CONTEXT_DEFINED
#define OVR_CONTEXT_DEFINED
typedef struct ovrAudioContext_* ovrAudioContext;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1) // Make sure all structs are packed for consistency.

/***********************************************************************************/
/* Geometry API */

/** \brief A handle to a material that applies filtering to reflected and transmitted sound. 0/NULL/nullptr represent an invalid handle. */
typedef struct ovrAudioMaterial_* ovrAudioMaterial;

/** \brief An enumeration of the scalar types supported for geometry data. */
typedef enum {
  ovrAudioScalarType_Int8,
  ovrAudioScalarType_UInt8,
  ovrAudioScalarType_Int16,
  ovrAudioScalarType_UInt16,
  ovrAudioScalarType_Int32,
  ovrAudioScalarType_UInt32,
  ovrAudioScalarType_Int64,
  ovrAudioScalarType_UInt64,
  ovrAudioScalarType_Float16,
  ovrAudioScalarType_Float32,
  ovrAudioScalarType_Float64,
} ovrAudioScalarType;

/** \brief The type of mesh face that is used to define geometry.
 *
 * For all face types, the vertices should be provided such that they are in counter-clockwise
 * order when the face is viewed from the front. The vertex order is used to determine the
 * surface normal orientation.
 */
typedef enum {
  /** \brief A face type that is defined by 3 vertex indices. */
  ovrAudioFaceType_Triangles = 0,
  /** \brief A face type that is defined by 4 vertex indices. The vertices are assumed to be coplanar. */
  ovrAudioFaceType_Quads = 1,
  ovrAudioFaceType_COUNT
} ovrAudioFaceType;

/** \brief The properties for audio materials. All properties are frequency dependent. */
typedef enum {
  /** \brief The fraction of sound arriving at a surface that is absorbed by the material.
   *
   * This value is in the range 0 to 1, where 0 indicates a perfectly reflective material, and
   * 1 indicates a perfectly absorptive material. Absorption is inversely related to the reverberation time,
   * and has the strongest impact on the acoustics of an environment. The default absorption is 0.1.
   */
  ovrAudioMaterialProperty_Absorption = 0,
  /** \brief The fraction of sound arriving at a surface that is transmitted through the material.
   *
   * This value is in the range 0 to 1, where 0 indicates a material that is acoustically opaque,
   * and 1 indicates a material that is acoustically transparent.
   * To preserve energy in the simulation, the following condition must hold: (1 - absorption + transmission) <= 1
   * If this condition is not met, the transmission and absorption coefficients will be modified to
   * enforce energy conservation. The default transmission is 0.
   */
  ovrAudioMaterialProperty_Transmission = 1,
  /** \brief The fraction of sound arriving at a surface that is scattered.
   *
   * This property in the range 0 to 1 controls how diffuse the reflections are from a surface,
   * where 0 indicates a perfectly specular reflection and 1 indicates a perfectly diffuse reflection.
   * The default scattering is 0.5.
   */
  ovrAudioMaterialProperty_Scattering = 2,
  ovrAudioMaterialProperty_COUNT
} ovrAudioMaterialProperty;

/** \brief A struct that is used to provide the vertex data for a mesh. */
typedef struct {
  /** \brief A pointer to a buffer of vertex data with the format described in this structure. This cannot be null. */
  const void* vertices;
  /** \brief The offset in bytes of the 0th vertex within the buffer. */
  size_t byteOffset;
  /** \brief The number of vertices that are contained in the buffer. */
  size_t vertexCount;
  /** \brief If non-zero, the stride in bytes between consecutive vertices. */
  size_t vertexStride;
  /** \brief The primitive type of vertex coordinates. Each vertex is defined by 3 consecutive values of this type. */
  ovrAudioScalarType vertexType;
} ovrAudioMeshVertices;

/** \brief A struct that is used to provide the index data for a mesh. */
typedef struct {
  /** \brief A pointer to a buffer of index data with the format described in this structure. This cannot be null. */
  const void* indices;
  /** \brief The offset in bytes of the 0th index within the buffer. */
  size_t byteOffset;
  /** \brief The total number of indices that are contained in the buffer. */
  size_t indexCount;
  /** \brief The primitive type of the indices in the buffer. This must be an integer type. */
  ovrAudioScalarType indexType;
} ovrAudioMeshIndices;

/** \brief A struct that defines a grouping of mesh faces and the material that should be applied to the faces. */
typedef struct {
  /** \brief The offset in the index buffer of the first index in the group. */
  size_t indexOffset;
  /** \brief The number of faces that this group uses from the index buffer.
   *
   * The number of bytes read from the index buffer for the group is determined by the formula:
   * (faceCount)*(verticesPerFace)*(bytesPerIndex)
   */
  size_t faceCount;
  /** \brief The type of face that the group uses. This determines how many indices are needed to define a face. */
  ovrAudioFaceType faceType;
  /** \brief A handle to the material that should be assigned to the group. If equal to 0/NULL/nullptr, a default material is used instead.
   */
  ovrAudioMaterial material;
} ovrAudioMeshGroup;

/** \brief A struct that completely defines an audio mesh. */
typedef struct {
  /** \brief The vertices that the mesh uses. */
  ovrAudioMeshVertices vertices;
  /** \brief The indices that the mesh uses. */
  ovrAudioMeshIndices indices;
  /** \brief A pointer to an array of ovrAudioMeshGroup that define the material groups in the mesh.
   *
   * The size of the array must be at least groupCount. This cannot be null.
   */
  const ovrAudioMeshGroup* groups;
  /** \brief The number of groups that are part of the mesh. */
  size_t groupCount;
} ovrAudioMesh;

/** \brief The boolean flags for an audio object. */
typedef enum {
  /** \brief If set, the object is used within the simulation and impacts the computed acoustics. */
  ovrAudioObjectFlag_Enabled = (1 << 0),
  /** \brief If set, the object is assumed to never move or change geometry. The context may use this flag as a hint to optimize the
   simulation. */
  ovrAudioObjectFlag_Static = (1 << 1),
} ovrAudioObjectFlags;

/** \brief A handle to geometry that sound interacts with. 0/NULL/nullptr represent an invalid handle. */
typedef struct ovrAudioGeometry_* ovrAudioGeometry;

OVRA_EXPORT ovrResult ovrAudio_SetPropagationQuality(ovrAudioContext context, float quality);
OVRA_EXPORT ovrResult ovrAudio_SetPropagationThreadAffinity(ovrAudioContext context, uint64_t cpuMask);

OVRA_EXPORT ovrResult ovrAudio_CreateAudioGeometry(ovrAudioContext context, ovrAudioGeometry* geometry);
OVRA_EXPORT ovrResult ovrAudio_DestroyAudioGeometry(ovrAudioGeometry geometry);
OVRA_EXPORT ovrResult ovrAudio_AudioGeometrySetObjectFlag(ovrAudioGeometry geometry, ovrAudioObjectFlags flag, int32_t enabled);

OVRA_EXPORT ovrResult ovrAudio_AudioGeometryUploadMesh(
    ovrAudioGeometry geometry,
    const ovrAudioMesh* mesh /*, const ovrAudioMeshSimplificationParameters* simplification*/);
OVRA_EXPORT ovrResult ovrAudio_AudioGeometryUploadMeshArrays(
    ovrAudioGeometry geometry,
    const void* vertices,
    size_t verticesByteOffset,
    size_t vertexCount,
    size_t vertexStride,
    ovrAudioScalarType vertexType,
    const void* indices,
    size_t indicesByteOffset,
    size_t indexCount,
    ovrAudioScalarType indexType,
    const ovrAudioMeshGroup* groups,
    size_t groupCount /*, const ovrAudioMeshSimplificationParameters* simplification*/);

OVRA_EXPORT ovrResult ovrAudio_AudioGeometrySetTransform(ovrAudioGeometry geometry, const float matrix4x4[16]);
OVRA_EXPORT ovrResult ovrAudio_AudioGeometryGetTransform(const ovrAudioGeometry geometry, float matrix4x4[16]);

OVRA_EXPORT ovrResult ovrAudio_AudioGeometryWriteMeshFile(const ovrAudioGeometry geometry, const char* filePath);
OVRA_EXPORT ovrResult ovrAudio_AudioGeometryReadMeshFile(ovrAudioGeometry geometry, const char* filePath);
OVRA_EXPORT ovrResult ovrAudio_AudioGeometryReadMeshMemory(ovrAudioGeometry geometry, const int8_t* data, uint64_t dataLength);

OVRA_EXPORT ovrResult ovrAudio_AudioGeometryWriteMeshFileObj(const ovrAudioGeometry geometry, const char* filePath);
/** \brief If the geometry is a mesh, copy the mesh vertices/indices into the output pointers.
 *
 * This function should be used by first calling it with vertices/indices == nullptr, and with
 * valid locations for numVertices/numTriangles. The function will write the size of the required
 * arrays to numVertices/numTriangles. Then, after allocating storage (3*numVertices*sizeof(float) + 3*numTriangles*sizeof(uint32_t)),
 * call the function again, passing in the pointers to the mesh storage, as well as the size of the
 * storage arrays. The function will copy the mesh data to the arrays.
 */
OVRA_EXPORT ovrResult ovrAudio_AudioGeometryGetSimplifiedMesh(
    const ovrAudioGeometry geometry,
    float* vertices,
    uint32_t* numVertices,
    uint32_t* indices,
    uint32_t* numTriangles);

/** \brief This function is the same as ovrAudio_AudioGeometryGetSimplifiedMesh, but it also returns the material indices for each triangle.
 * materialIndices is an array of length numTriangles, and is populated with the material indices for each triangle.
 * Go to ovrAudio_AudioGeometryGetSimplifiedMesh for more details:
 * \see ovrAudio_AudioGeometryGetSimplifiedMesh
 */
OVRA_EXPORT ovrResult ovrAudio_AudioGeometryGetSimplifiedMeshWithMaterials(
    const ovrAudioGeometry geometry,
    float* vertices,
    uint32_t* numVertices,
    uint32_t* indices,
    uint32_t* materialIndices,
    uint32_t* numTriangles);

/***********************************************************************************/
/* Material API */

OVRA_EXPORT ovrResult ovrAudio_CreateAudioMaterial(ovrAudioContext context, ovrAudioMaterial* material);
OVRA_EXPORT ovrResult ovrAudio_DestroyAudioMaterial(ovrAudioMaterial material);

OVRA_EXPORT ovrResult
ovrAudio_AudioMaterialSetFrequency(ovrAudioMaterial material, ovrAudioMaterialProperty property, float frequency, float value);
OVRA_EXPORT ovrResult
ovrAudio_AudioMaterialGetFrequency(const ovrAudioMaterial material, ovrAudioMaterialProperty property, float frequency, float* value);
OVRA_EXPORT ovrResult ovrAudio_AudioMaterialReset(ovrAudioMaterial material, ovrAudioMaterialProperty property);

/***********************************************************************************/
/* Audio Scene IR types */

/** \brief A handle to an audio scene IR (impulse response). 0/NULL/nullptr represent an invalid handle.  */
typedef struct ovrAudioSceneIR_* ovrAudioSceneIR;

/** \brief Flags describing the status an audio scene IR. */
typedef enum {
  /** \brief A status flag indicating the IR has no data (i.e. was just created). */
  ovrAudioSceneIRStatus_Empty = 0,
  /** \brief A status flag indicating the scene has been spatially mapped (but IR not necessarily computed). */
  ovrAudioSceneIRStatus_Mapped = (1 << 0),
  /** \brief A status flag indicating the IR is ready for use in a runtime simulation (i.e. has been mapped and computed). */
  ovrAudioSceneIRStatus_Ready = (1 << 1) | ovrAudioSceneIRStatus_Mapped,
} ovrAudioSceneIRStatus;

/** \brief The boolean flags for an audio scene IR. */
typedef enum {
  /** \brief The flag value when no flags are set. */
  ovrAudioSceneIRFlags_None = 0,
  /** \brief If set, only objects that have the ovrAudioObjectFlag_Static flag set will be considered for the computation. */
  ovrAudioSceneIRFlag_StaticOnly = (1 << 0),
  /** \brief If set, no IRs will be calculated for points that are floating far above the floor (as determined by gravity vector). */
  ovrAudioSceneIRFlag_NoFloating = (1 << 1),
  /** \brief If set, no IRs will be calculated. The scene will be mapped but not simulated. */
  ovrAudioSceneIRFlag_MapOnly = (1 << 2),
  /** \brief If set, the scene will be preprocessed to support runtime diffraction. */
  ovrAudioSceneIRFlag_Diffraction = (1 << 3),
} ovrAudioSceneIRFlags;

/** \brief A function pointer that provides information about the current progress on a long-running operation.
 *
 * This function will be called periodically by long-running operations to report progress to the GUI.
 * The progress value is in the range [0,1] and indicates the approximate fraction of the operation
 * that has been performed so far, and could be used to display a progress bar.
 * The string is a NULL-terminated ASCII description of the current task.
 * The return value indicates whether or not to continue processing (non-zero -> continue, zero -> cancel).
 */
typedef uint32_t (*ovrAudioProgressCallback)(void* userData, const char* string, float progress);

/** \brief A struct containing callback functions used when computing a scene IR. */
typedef struct {
  /** \brief A pointer to arbitrary user data that is passed into the callback functions. */
  void* userData;
  /** \brief A pointer to function that reports the computation progress to a GUI. */
  ovrAudioProgressCallback progress;
} ovrAudioSceneIRCallbacks;

/** \brief A struct that describes how a scene IR is computed. */
typedef struct {
  /** \brief A value that should be set to the size in bytes of the ovrAudioSceneIRParameters structure.
   * This is used for backward/forward compatibility.
   */
  size_t thisSize; /* = sizeof(ovrAudioSceneIRParameters) */
  /** \brief A struct containing callback functions that receive notifications during computation. */
  ovrAudioSceneIRCallbacks callbacks;
  /** \brief The number of threads that should be used for doing the computation.
   * A value of 0 means to use the same number of threads as CPUs.
   * A value of 1 indicates that all work is done on the calling thread (no threads created).
   * This number will be clamped to no more than the number of CPUs.
   */
  size_t threadCount; /* = 0 */
  /** \brief The number of early reflections that should be stored in the scene IR.
   * Increasing this value increases the size of the IR data, as well as the quality.
   */
  size_t reflectionCount; /* = 6 */
  /** \brief Flags that describe how a scene IR should be computed. */
  ovrAudioSceneIRFlags flags; /* = ovrAudioSceneIRFlags_NoFloating */
  /** \brief The minimum point placement resolution, expressed in meters.
   * This determines the smallest spacing of points in the scene, as well as
   * the smallest space that will be considered as part of the precomputation.
   * This should be a little bit smaller than the smallest possible space
   * that the listener can traverse.
   * Note that decreasing this value doesn't necessarily result in more densely-sampled points -
   * the number of points is more clostly related to the number of distinct acoustic spaces
   * in the scene.
   */
  float minResolution; /* = 1.0f */
  /** \brief The maximum point placement resolution, expressed in meters.
   * This determines the largest spacing of points in the scene.
   * Decreasing this value improves the quality for large open scenes,
   * but also increases the precomputation time and storage required.
   */
  float maxResolution; /* = 10.0f */
  /** \brief The typical height in meters of the listener's head above floor surfaces.
   * This is used to determine where to vertically place probe points in the scene.
   * The quality will be best for sources and listeners that are around this height.
   */
  float headHeight; /* = 1.5f */
  /** \brief The maximum height in meters of a probe point above floor surfaces.
   * Increase this value to include more points floating in the air.
   */
  float maxHeight; /* = 3.0f */
  /** \brief A 3D unit vector indicating the downward direction in the scene.
   * This is used to cull probe points that are irrelevant because they are too high
   * above the floor surface.
   * In a Y-up world, this should be {0,-1,0}, while Z-up would be {0,0,-1}.
   */
  float gravityVector[3]; /* = { 0, -1, 0 } */

} ovrAudioSceneIRParameters;

/** \brief A function pointer that reads bytes from an arbitrary source and places them into the output byte array.
 *
 * The function should return the number of bytes that were successfully read, or 0 if there was an error.
 */
typedef size_t (*ovrAudioSerializerReadCallback)(void* userData, void* bytes, size_t byteCount);
/** \brief A function pointer that writes bytes to an arbitrary destination.
 *
 * The function should return the number of bytes that were successfully written, or 0 if there was an error.
 */
typedef size_t (*ovrAudioSerializerWriteCallback)(void* userData, const void* bytes, size_t byteCount);
/** \brief A function pointer that seeks within the data stream.
 *
 * The function should seek by the specified signed offset relative to the current stream position.
 * The function should return the actual change in stream position. Return 0 if there is an error or seeking is not supported.
 */
typedef int64_t (*ovrAudioSerializerSeekCallback)(void* userData, int64_t seekOffset);

/** \brief A structure that contains function pointers to reading/writing data to an arbitrary source/sink. */
typedef struct {
  /** \brief A function pointer that reads bytes from an arbitrary source. This pointer may be null if only writing is required. */
  ovrAudioSerializerReadCallback read;
  /** \brief A function pointer that writes bytes to an arbitrary destination. This pointer may be null if only reading is required. */
  ovrAudioSerializerWriteCallback write;
  /** \brief A function pointer that seeks within the data stream. This pointer may be null if seeking is not supported. */
  ovrAudioSerializerSeekCallback seek;
  /** \brief A pointer to user-defined data that will be passed in as the first argument to the serialization functions. */
  void* userData;
} ovrAudioSerializer;

OVRA_EXPORT ovrResult ovrAudio_CreateAudioSceneIR(ovrAudioContext context, ovrAudioSceneIR* sceneIR);
OVRA_EXPORT ovrResult ovrAudio_DestroyAudioSceneIR(ovrAudioSceneIR sceneIR);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRSetEnabled(ovrAudioSceneIR sceneIR, int enabled);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRGetEnabled(ovrAudioSceneIR sceneIR, int* enabled);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRGetStatus(ovrAudioSceneIR sceneIR, ovrAudioSceneIRStatus* status);
OVRA_EXPORT ovrResult ovrAudio_InitializeAudioSceneIRParameters(ovrAudioSceneIRParameters* parameters);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRCompute(ovrAudioSceneIR sceneIR, ovrAudioSceneIRParameters* parameters);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRComputeCustomPoints(
    ovrAudioSceneIR sceneIR,
    const float* points,
    size_t pointCount,
    ovrAudioSceneIRParameters* parameters);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRGetPointCount(ovrAudioSceneIR sceneIR, size_t* pointCount);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRGetPoints(ovrAudioSceneIR sceneIR, float* points, size_t maxPointCount);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRSetTransform(ovrAudioSceneIR sceneIR, const float matrix4x4[16]);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRGetTransform(const ovrAudioSceneIR sceneIR, float matrix4x4[16]);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRWriteFile(ovrAudioSceneIR sceneIR, const char* filePath);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRReadFile(ovrAudioSceneIR sceneIR, const char* filePath);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRReadMemory(ovrAudioSceneIR sceneIR, const int8_t* data, uint64_t dataLength);
OVRA_EXPORT ovrResult ovrAudio_AudioSceneIRWriteMeshFileObj(const ovrAudioSceneIR sceneIR, const char* filePath);

/***********************************************************************************/
/* Serialization API */

OVRA_EXPORT ovrResult ovrAudio_AudioGeometryWriteMeshData(const ovrAudioGeometry geometry, const ovrAudioSerializer* serializer);
OVRA_EXPORT ovrResult ovrAudio_AudioGeometryReadMeshData(ovrAudioGeometry geometry, const ovrAudioSerializer* serializer);

/***********************************************************************************/
/* Control Zone API */

/** \brief A handle to a zone that overrides or modifies the acoustic IR within. 0/NULL/nullptr represent an invalid handle.  */
typedef struct ovrAudioControlZone_* ovrAudioControlZone;

/** \brief The properties of an audio control Zone. */
typedef enum {
  /** \brief A frequency-dependent property describing the reverberation time adjustment in the Zone.
   *
   * The value represents a signed additive adjustment to the simulated reverb time (units: seconds).
   */
  ovrAudioControlZoneProperty_RT60,
  /** \brief A frequency-dependent property describing the reverberation level adjustment in the Zone.
   *
   * The value represents a signed additive adjustment to the simulated reverb level (units: decibels).
   */
  ovrAudioControlZoneProperty_ReverbLevel,
  ovrAudioControlZoneProperty_COUNT
} ovrAudioControlZoneProperty;

/** \brief Create a new audio control Zone handle with the default initial state. */
OVRA_EXPORT ovrResult ovrAudio_CreateControlZone(ovrAudioContext context, ovrAudioControlZone* control);
/** \brief Destroy an audio control Zone handle. The control Zone must not be referenced by a scene when it is destroyed. */
OVRA_EXPORT ovrResult ovrAudio_DestroyControlZone(ovrAudioControlZone control);

/** \brief Set whether or not the control Zone is enabled. */
OVRA_EXPORT ovrResult ovrAudio_ControlZoneSetEnabled(ovrAudioControlZone control, int enabled);
/** \brief Get whether or not the control Zone is enabled. */
OVRA_EXPORT ovrResult ovrAudio_ControlZoneGetEnabled(const ovrAudioControlZone control, int* enabled);

/** \brief Set a 4x4 column-major matrix encoding the homogeneous coordinate transformation from control Zone-local-space to world space.
 */
OVRA_EXPORT ovrResult ovrAudio_ControlZoneSetTransform(ovrAudioControlZone control, const float matrix4x4[16]);
/** \brief Get a 4x4 column-major matrix encoding the homogeneous coordinate transformation from control Zone-local-space to world space.
 */
OVRA_EXPORT ovrResult ovrAudio_ControlZoneGetTransform(const ovrAudioControlZone control, float matrix4x4[16]);

/** \brief Set the local size of the control Zone box along each axis. The box is centered at the local origin. */
OVRA_EXPORT ovrResult ovrAudio_ControlZoneSetBox(ovrAudioControlZone control, float sizeX, float sizeY, float sizeZ);
/** \brief Get the local size of the control Zone box along each axis. The box is centered at the local origin. */
OVRA_EXPORT ovrResult ovrAudio_ControlZoneGetBox(const ovrAudioControlZone control, float* sizeX, float* sizeY, float* sizeZ);

/** \brief Set the distance margin that is used to fade out the Zone's influence near its boundaries. */
OVRA_EXPORT ovrResult ovrAudio_ControlZoneSetFadeDistance(ovrAudioControlZone control, float fadeX, float fadeY, float fadeZ);
/** \brief Get the distance margin that is used to fade out the Zone's influence near its boundaries. */
OVRA_EXPORT ovrResult ovrAudio_ControlZoneGetFadeDistance(const ovrAudioControlZone control, float* fadeX, float* fadeY, float* fadeZ);

/** \brief Set the value of a frequency-dependent control Zone property at a specific frequency. If there is a previous value at that
 * frequency it is replaced. */
OVRA_EXPORT ovrResult
ovrAudio_ControlZoneSetFrequency(ovrAudioControlZone control, ovrAudioControlZoneProperty property, float frequency, float value);
/** \brief Remove all frequency data points from the specified control Zone property. */
OVRA_EXPORT ovrResult ovrAudio_ControlZoneReset(ovrAudioControlZone control, ovrAudioControlZoneProperty property);

/** \brief Set the value of a frequency-dependent Room control property at a specific frequency. If there is a previous value at that
 * frequency it is replaced. */
OVRA_EXPORT ovrResult
ovrAudio_SetRoomControlValue(ovrAudioContext Context, ovrAudioControlZoneProperty property, float frequency, float value);

OVRA_EXPORT ovrResult ovrAudio_ResetRoomControl(ovrAudioContext Context, ovrAudioControlZoneProperty property);

/***********************************************************************************/
/* Occlusion API */

/// The strength of the occlusion when the source is not directly visible, in the range [0,1].
///
/// \param context[in] context to use
/// \param sound[in] index of sound (0..NumSources-1)
/// \param value[in] A value of 1 means full effect (realistic occlusion), while 0 means no occlusion occurs.
///
OVRA_EXPORT ovrResult ovrAudio_SetSourceOcclusionIntensity(ovrAudioContext context, int sound, float value);

/// The strength of the occlusion when the source is not directly visible, in the range [0,1].
///
/// \param context[in] context to use
/// \param sound[in] index of sound (0..NumSources-1)
/// \param value[in] A value of 1 means full effect (realistic occlusion), while 0 means no occlusion occurs.
///
OVRA_EXPORT ovrResult ovrAudio_GetSourceOcclusionIntensity(ovrAudioContext context, int sound, float* value);

/***********************************************************************************/
/* Diffraction API */

/** \brief Flags to describe how to render a mesh */
typedef enum {
  ovrAudioMeshFlags_None = 0,
  /** \brief Turning on Mesh Simplification will reduce the resource usage of geometry propagation at the cost of audio quality. */
  ovrAudioMeshFlags_enableMeshSimplification = (1 << 0),
  /** \brief If diffraction is enabled, the geometry will support real-time diffraction. Note, if this is disabled and you try
   * to turn on real time diffraction, you won't hear a difference as the geometry did not render to support geometry. */
  ovrAudioMeshFlags_enableDiffraction = (1 << 1),
} ovrAudioMeshFlags;

/** \brief A struct that describes how a mesh should be simplified. */
typedef struct {
  /** The size of this structure. You must set this equal to sizeof(ovrAudioMeshSimplification) This will ensure version compatibility */
  size_t thisSize;
  ovrAudioMeshFlags flags;
  /** \brief The local unit scale factor for the mesh (the factor that converts from mesh-local coordinates to coordinates in meters).
   * The other length quantities in this struct are converted to mesh-local coordinates using this value. */
  float unitScale; /* = 1.0f */
  /** \brief The maximum allowed error due to simplification, expressed as a distance in meters. */
  float maxError; /* = 0.1f */
  /** \brief The minimum angle (degrees) that there must be between two adjacent face normals for their edge to be marked as diffracting. */
  float minDiffractionEdgeAngle; /* = 1.0f */
  /** \brief The minimum length in meters that an edge should have for it to be marked as diffracting. */
  float minDiffractionEdgeLength; /* = 0.01f */
  /** \brief The maximum distance in meters that a diffraction flag extends out from the edge. */
  float flagLength; /* = 1.0f */
  /** \brief The number of threads that should be used for processing the mesh.
   * A value of 0 means to use the same number of threads as CPUs.
   * A value of 1 indicates that all work is done on the calling thread (no threads created).
   * This number will be clamped to no more than the number of CPUs.
   */
  size_t threadCount; /* = 0 */
} ovrAudioMeshSimplification;

/// Set the diffraction order to trade off between diffraction quality and resource usage
///
/// \param value[in] The number of diffraction events that can occur along a single sound propagation path
///
OVRA_EXPORT ovrResult ovrAudio_SetDiffractionOrder(ovrAudioContext context, int value);

OVRA_EXPORT ovrResult ovrAudio_AudioGeometryUploadSimplifiedMesh(
    ovrAudioGeometry geometry,
    const ovrAudioMesh* mesh,
    const ovrAudioMeshSimplification* simplification);

OVRA_EXPORT ovrResult ovrAudio_AudioGeometryUploadSimplifiedMeshArrays(
    ovrAudioGeometry geometry,
    const void* vertices,
    size_t verticesByteOffset,
    size_t vertexCount,
    size_t vertexStride,
    ovrAudioScalarType vertexType,
    const void* indices,
    size_t indicesByteOffset,
    size_t indexCount,
    ovrAudioScalarType indexType,
    const ovrAudioMeshGroup* groups,
    size_t groupCount,
    const ovrAudioMeshSimplification* simplification);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif // OVR_Audio_Propagation_h
