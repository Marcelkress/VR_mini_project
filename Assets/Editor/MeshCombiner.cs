using UnityEngine;
using UnityEditor;
using UnityEngine.Rendering;
using System.Collections.Generic;
using System.IO;

public class MeshCombiner
{
    private const string MenuPath = "Assets/Combine Meshes in Prefab";

    [MenuItem(MenuPath)]
    private static void CombineMeshesInPrefab()
    {
        GameObject prefab = Selection.activeObject as GameObject;
        if (prefab == null || !PrefabUtility.IsPartOfPrefabAsset(prefab))
        {
            EditorUtility.DisplayDialog("No Prefab Selected", "Please select a prefab asset in the Project view.", "OK");
            return;
        }

        GameObject instance = PrefabUtility.InstantiatePrefab(prefab) as GameObject;
        if (instance == null)
        {
            EditorUtility.DisplayDialog("Instantiation Failed", "Unable to instantiate the selected prefab.", "OK");
            return;
        }

        try
        {
            MeshFilter[] meshFilters = instance.GetComponentsInChildren<MeshFilter>(true);
            if (meshFilters.Length == 0)
            {
                EditorUtility.DisplayDialog("No Meshes Found", "The selected prefab does not contain any MeshFilters.", "OK");
                return;
            }

            Dictionary<Material, List<CombineInstance>> materialToInstances = new Dictionary<Material, List<CombineInstance>>();

            Matrix4x4 worldToLocal = instance.transform.worldToLocalMatrix;

            foreach (MeshFilter meshFilter in meshFilters)
            {
                MeshRenderer meshRenderer = meshFilter.GetComponent<MeshRenderer>();
                if (meshRenderer == null)
                    continue;

                Mesh sourceMesh = meshFilter.sharedMesh;
                if (sourceMesh == null)
                    continue;

                Material[] materials = meshRenderer.sharedMaterials;
                int subMeshCount = Mathf.Min(sourceMesh.subMeshCount, materials.Length);

                for (int subMeshIndex = 0; subMeshIndex < subMeshCount; subMeshIndex++)
                {
                    Material material = materials[subMeshIndex];
                    if (material == null)
                        continue;

                    CombineInstance combineInstance = new CombineInstance
                    {
                        mesh = sourceMesh,
                        subMeshIndex = subMeshIndex,
                        transform = worldToLocal * meshRenderer.transform.localToWorldMatrix
                    };

                    if (!materialToInstances.TryGetValue(material, out List<CombineInstance> combineList))
                    {
                        combineList = new List<CombineInstance>();
                        materialToInstances.Add(material, combineList);
                    }

                    combineList.Add(combineInstance);
                }
            }

            if (materialToInstances.Count == 0)
            {
                EditorUtility.DisplayDialog("No Valid Mesh Data", "Unable to find matching meshes and materials to combine.", "OK");
                return;
            }

            List<CombineInstance> subMeshBatches = new List<CombineInstance>();
            List<Material> materialsList = new List<Material>();

            foreach (KeyValuePair<Material, List<CombineInstance>> kvp in materialToInstances)
            {
                Mesh combinedSubMesh = new Mesh
                {
                    indexFormat = IndexFormat.UInt32,
                    name = kvp.Key != null ? $"{prefab.name}_{kvp.Key.name}_SubMesh" : $"{prefab.name}_SubMesh"
                };

                combinedSubMesh.CombineMeshes(kvp.Value.ToArray(), true, true, false);
                combinedSubMesh.RecalculateBounds();

                subMeshBatches.Add(new CombineInstance
                {
                    mesh = combinedSubMesh,
                    subMeshIndex = 0,
                    transform = Matrix4x4.identity
                });

                materialsList.Add(kvp.Key);
            }

            Mesh finalMesh = new Mesh
            {
                indexFormat = IndexFormat.UInt32,
                name = prefab.name + "_CombinedMesh"
            };

            finalMesh.CombineMeshes(subMeshBatches.ToArray(), false, false, false);
            finalMesh.RecalculateBounds();

            GenerateSecondaryUVs(finalMesh);
            MeshUtility.Optimize(finalMesh);

            string defaultAssetName = prefab.name + "_CombinedMesh";
            string meshAssetPath = EditorUtility.SaveFilePanelInProject(
                "Save Combined Mesh",
                defaultAssetName,
                "asset",
                "Choose where to store the combined mesh asset.");

            if (string.IsNullOrEmpty(meshAssetPath))
            {
                EditorUtility.DisplayDialog("Cancelled", "Mesh combining cancelled.", "OK");
                return;
            }

            meshAssetPath = AssetDatabase.GenerateUniqueAssetPath(meshAssetPath);

            AssetDatabase.CreateAsset(finalMesh, meshAssetPath);

            foreach (CombineInstance batch in subMeshBatches)
            {
                Object.DestroyImmediate(batch.mesh);
            }

            MeshRenderer templateRenderer = FindTemplateRenderer(meshFilters);

            string prefabAssetPath = AssetDatabase.GenerateUniqueAssetPath(Path.ChangeExtension(meshAssetPath, ".prefab"));
            GameObject combinedObject = new GameObject(prefab.name + "_Combined");

            try
            {
                combinedObject.transform.SetPositionAndRotation(instance.transform.position, instance.transform.rotation);
                combinedObject.transform.localScale = instance.transform.localScale;
                combinedObject.tag = instance.tag;
                combinedObject.layer = instance.layer;

                MeshFilter combinedFilter = combinedObject.AddComponent<MeshFilter>();
                combinedFilter.sharedMesh = finalMesh;

                MeshRenderer combinedRenderer = combinedObject.AddComponent<MeshRenderer>();
                combinedRenderer.sharedMaterials = materialsList.ToArray();

                ApplyRendererSettings(templateRenderer, combinedRenderer);
                ApplyStaticFlags(templateRenderer, combinedObject);

                PrefabUtility.SaveAsPrefabAsset(combinedObject, prefabAssetPath);
            }
            finally
            {
                Object.DestroyImmediate(combinedObject);
            }

            AssetDatabase.SaveAssets();
            AssetDatabase.Refresh();

            EditorUtility.DisplayDialog("Success", $"Combined mesh saved to:\n{meshAssetPath}", "OK");
        }
        finally
        {
            Object.DestroyImmediate(instance);
        }
    }

    private static MeshRenderer FindTemplateRenderer(MeshFilter[] meshFilters)
    {
        foreach (MeshFilter filter in meshFilters)
        {
            MeshRenderer renderer = filter.GetComponent<MeshRenderer>();
            if (renderer != null)
                return renderer;
        }

        return null;
    }

    private static void ApplyRendererSettings(MeshRenderer template, MeshRenderer target)
    {
        target.lightmapIndex = -1;
        target.lightmapScaleOffset = Vector4.zero;
        target.realtimeLightmapIndex = -1;
        target.realtimeLightmapScaleOffset = Vector4.zero;

        if (template == null)
            return;

        target.shadowCastingMode = template.shadowCastingMode;
        target.receiveShadows = template.receiveShadows;
        target.motionVectorGenerationMode = template.motionVectorGenerationMode;
        target.lightProbeUsage = template.lightProbeUsage;
        target.reflectionProbeUsage = template.reflectionProbeUsage;
        target.allowOcclusionWhenDynamic = template.allowOcclusionWhenDynamic;
        target.probeAnchor = template.probeAnchor;
        target.rendererPriority = template.rendererPriority;
        target.sortingLayerID = template.sortingLayerID;
        target.sortingOrder = template.sortingOrder;
        target.enabled = template.enabled;

    target.receiveGI = template.receiveGI;
    target.scaleInLightmap = template.scaleInLightmap;
    target.stitchLightmapSeams = template.stitchLightmapSeams;

#if UNITY_2020_2_OR_NEWER
        target.rayTracingMode = template.rayTracingMode;
#endif
    }

    private static void ApplyStaticFlags(MeshRenderer template, GameObject combinedObject)
    {
        if (template != null)
        {
            StaticEditorFlags flags = GameObjectUtility.GetStaticEditorFlags(template.gameObject);
            GameObjectUtility.SetStaticEditorFlags(combinedObject, flags);
        }
        else
        {
            const StaticEditorFlags defaultFlags = StaticEditorFlags.BatchingStatic |
                                                   StaticEditorFlags.ContributeGI |
                                                   StaticEditorFlags.OccludeeStatic |
                                                   StaticEditorFlags.OccluderStatic;
            GameObjectUtility.SetStaticEditorFlags(combinedObject, defaultFlags);
        }
    }

    private static void GenerateSecondaryUVs(Mesh mesh)
    {
        if (mesh == null)
            return;

        UnwrapParam unwrapParams;
        UnwrapParam.SetDefaults(out unwrapParams);
        unwrapParams.packMargin = 0.004f;
        unwrapParams.hardAngle = 60f;
        unwrapParams.angleError = 15f;
        unwrapParams.areaError = 15f;

        Unwrapping.GenerateSecondaryUVSet(mesh, unwrapParams);
    }
}