using UnityEngine;
using UnityEditor;
using UnityEngine.Rendering;
using System.Collections.Generic;
using System.IO;
using System.Linq;

public class AdvancedURPMaterialCombiner : EditorWindow
{
    private const string CustomPropertyOption = "Custom...";
    private TexturePropertySetting[] texturePropertySettings;
    private string[] availableTexturePropertyNames = new string[0];
    private Material referenceMaterial;
    private Shader referenceShader;

    [System.Serializable]
    private class TexturePropertySetting
    {
        public string label;
        public string defaultPropertyName;
        public Texture2D fallbackTexture;
        public string propertyName;
        public bool include;
        public string atlasFileName;
        public bool sRGB;
        public TextureImporterType importerType;
        public readonly Dictionary<Material, Texture2D> textures = new Dictionary<Material, Texture2D>();

        public TexturePropertySetting(string label, string defaultPropertyName, Texture2D fallbackTexture, string atlasFileName, bool sRGB, TextureImporterType importerType)
        {
            this.label = label;
            this.defaultPropertyName = defaultPropertyName;
            this.fallbackTexture = fallbackTexture;
            this.propertyName = defaultPropertyName;
            this.include = true;
            this.atlasFileName = atlasFileName;
            this.sRGB = sRGB;
            this.importerType = importerType;
        }

        public string ResolvedPropertyName => string.IsNullOrWhiteSpace(propertyName) ? defaultPropertyName : propertyName.Trim();

        public void ResetTextures()
        {
            textures.Clear();
        }
    }

    [MenuItem("Tools/Advanced Combine Materials into One (URP)")]
    public static void ShowWindow()
    {
        EditorWindow.GetWindow(typeof(AdvancedURPMaterialCombiner));
    }

    private void OnEnable()
    {
        InitializeTextureSettings();
        RefreshSelectionContext();
        Selection.selectionChanged += RefreshSelectionContext;
    }

    private void OnDisable()
    {
        Selection.selectionChanged -= RefreshSelectionContext;
    }

    private void OnGUI()
    {
        GUILayout.Label("Advanced Combine Materials into One (URP)", EditorStyles.boldLabel);

        DrawSelectionInfo();
        DrawTexturePropertySettingsUI();

        if (GUILayout.Button("Combine Materials on Selected Objects"))
        {
            CombineMaterials();
        }
    }

    private void DrawSelectionInfo()
    {
        if (referenceMaterial == null)
        {
            EditorGUILayout.HelpBox("Select at least one object with a renderer to preview available shader properties.", MessageType.Info);
            return;
        }

        EditorGUILayout.LabelField("Detected Shader", referenceShader != null ? referenceShader.name : "None");

        if (availableTexturePropertyNames.Length == 0)
        {
            EditorGUILayout.HelpBox("The detected shader exposes no texture properties. You can still enter custom property names manually.", MessageType.Warning);
        }
    }

    private void DrawTexturePropertySettingsUI()
    {
        if (texturePropertySettings == null)
            return;

        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Texture Property Mapping", EditorStyles.boldLabel);

        foreach (var setting in texturePropertySettings)
        {
            DrawTexturePropertySetting(setting);
        }
    }

    private void DrawTexturePropertySetting(TexturePropertySetting setting)
    {
        EditorGUILayout.BeginVertical(GUI.skin.box);
        setting.include = EditorGUILayout.ToggleLeft($"Include {setting.label}", setting.include);

        if (setting.include)
        {
            string resolvedProperty = setting.ResolvedPropertyName;

            if (availableTexturePropertyNames.Length > 0)
            {
                var popupOptions = new List<string> { CustomPropertyOption };
                popupOptions.AddRange(availableTexturePropertyNames);

                int selectedIndex = 0;
                if (!string.IsNullOrEmpty(resolvedProperty))
                {
                    int existingIndex = System.Array.IndexOf(availableTexturePropertyNames, resolvedProperty);
                    if (existingIndex >= 0)
                    {
                        selectedIndex = existingIndex + 1;
                    }
                }

                selectedIndex = EditorGUILayout.Popup("Shader Property", selectedIndex, popupOptions.ToArray());

                if (selectedIndex == 0)
                {
                    string newName = EditorGUILayout.TextField("Property Name", string.IsNullOrEmpty(setting.propertyName) ? setting.defaultPropertyName : setting.propertyName);
                    setting.propertyName = string.IsNullOrWhiteSpace(newName) ? setting.defaultPropertyName : newName.Trim();
                }
                else
                {
                    setting.propertyName = availableTexturePropertyNames[selectedIndex - 1];
                }
            }
            else
            {
                string newName = EditorGUILayout.TextField("Property Name", string.IsNullOrEmpty(setting.propertyName) ? setting.defaultPropertyName : setting.propertyName);
                setting.propertyName = string.IsNullOrWhiteSpace(newName) ? setting.defaultPropertyName : newName.Trim();
            }

            setting.fallbackTexture = (Texture2D)EditorGUILayout.ObjectField("Fallback Texture", setting.fallbackTexture, typeof(Texture2D), false);
        }

        EditorGUILayout.EndVertical();
    }

    private void InitializeTextureSettings()
    {
        if (texturePropertySettings != null)
            return;

        texturePropertySettings = new[]
        {
            new TexturePropertySetting("Base Color Map", "_BaseMap", Texture2D.whiteTexture, "AlbedoAtlas.png", true, TextureImporterType.Default),
            new TexturePropertySetting("Normal Map", "_BumpMap", Texture2D.normalTexture, "NormalAtlas.png", false, TextureImporterType.NormalMap),
            new TexturePropertySetting("Metallic Map", "_MetallicGlossMap", Texture2D.blackTexture, "MetallicAtlas.png", false, TextureImporterType.Default)
        };
    }

    private void RefreshSelectionContext()
    {
        referenceMaterial = FindFirstMaterial(Selection.gameObjects);
        referenceShader = referenceMaterial != null ? referenceMaterial.shader : null;
        availableTexturePropertyNames = referenceShader != null ? GetTexturePropertyNames(referenceShader) : new string[0];

        if (referenceMaterial != null)
        {
            foreach (var setting in texturePropertySettings)
            {
                if (!setting.include)
                    continue;

                string resolved = setting.ResolvedPropertyName;

                if (!string.IsNullOrEmpty(resolved) && referenceMaterial.HasProperty(resolved))
                    continue;

                if (referenceMaterial.HasProperty(setting.defaultPropertyName))
                {
                    setting.propertyName = setting.defaultPropertyName;
                    continue;
                }

                string firstMatching = availableTexturePropertyNames.FirstOrDefault();
                if (!string.IsNullOrEmpty(firstMatching))
                {
                    setting.propertyName = firstMatching;
                }
            }
        }

        Repaint();
    }

    private Material FindFirstMaterial(GameObject[] objects)
    {
        foreach (var obj in objects)
        {
            Renderer renderer = obj.GetComponentInChildren<Renderer>(true);
            if (renderer == null)
                continue;

            foreach (var material in renderer.sharedMaterials)
            {
                if (material != null)
                    return material;
            }
        }

        return null;
    }

    private string[] GetTexturePropertyNames(Shader shader)
    {
        if (shader == null)
            return new string[0];

        int propertyCount = shader.GetPropertyCount();
        List<string> propertyNames = new List<string>();

        for (int i = 0; i < propertyCount; i++)
        {
            if (shader.GetPropertyType(i) == ShaderPropertyType.Texture)
            {
                propertyNames.Add(shader.GetPropertyName(i));
            }
        }

        return propertyNames.ToArray();
    }

    private void CombineMaterials()
    {
        GameObject[] selectedObjects = Selection.gameObjects;

        if (selectedObjects.Length == 0)
        {
            Debug.LogWarning("No objects selected.");
            return;
        }

        List<TexturePropertySetting> activeSettings = texturePropertySettings
            .Where(setting => setting.include && !string.IsNullOrWhiteSpace(setting.ResolvedPropertyName))
            .ToList();

        if (activeSettings.Count == 0)
        {
            Debug.LogWarning("Enable at least one texture property before combining materials.");
            return;
        }

        foreach (var setting in activeSettings)
        {
            setting.ResetTextures();
        }

        List<Material> materials = new List<Material>();
        List<Texture2D> processedTextures = new List<Texture2D>(); // To keep track of modified textures

        // Collect materials and their textures
        Material templateMaterial = CollectMaterialsAndTextures(selectedObjects, materials, activeSettings, processedTextures);

        if (templateMaterial == null || materials.Count == 0)
        {
            Debug.LogWarning("No compatible materials found. Ensure all selected renderers share the same shader.");
            return;
        }

        if (materials.Count != activeSettings[0].textures.Count)
        {
            Debug.LogWarning("Mismatch while collecting textures. Aborting to avoid incorrect atlas generation.");
            return;
        }

        // Create texture atlases
        Dictionary<TexturePropertySetting, Texture2D> atlases;
        Rect[] rects = CreateTextureAtlases(materials, activeSettings, out atlases);

        if (rects == null || atlases.Count == 0)
        {
            Debug.LogWarning("Failed to create texture atlases.");
            return;
        }

        // Save atlases as assets
        string atlasPath = "Assets/CombinedMaterialAtlases/";
        if (!Directory.Exists(atlasPath))
        {
            Directory.CreateDirectory(atlasPath);
        }

        Dictionary<TexturePropertySetting, string> atlasAssetPaths = new Dictionary<TexturePropertySetting, string>();
        foreach (var pair in atlases)
        {
            string sanitizedFileName = string.IsNullOrEmpty(pair.Key.atlasFileName)
                ? pair.Key.label.Replace(" ", string.Empty) + "Atlas.png"
                : pair.Key.atlasFileName;

            string fullPath = Path.Combine(atlasPath, sanitizedFileName);
            SaveTextureAsAsset(pair.Value, fullPath, pair.Key);
            atlasAssetPaths[pair.Key] = fullPath;
        }

        // Create new material based on the template shader
        Material newMaterial = CreateCombinedMaterial(templateMaterial, atlasAssetPaths);

        // Save the material as an asset
        string combinedMaterialName = string.IsNullOrEmpty(templateMaterial.name) ? "CombinedMaterial" : templateMaterial.name + "_Combined";
        string materialPath = AssetDatabase.GenerateUniqueAssetPath(Path.Combine(atlasPath, combinedMaterialName + ".mat"));
        AssetDatabase.CreateAsset(newMaterial, materialPath);

        // Adjust UVs and assign new material
        ProcessGameObjects(selectedObjects, materials, rects, newMaterial);

        // Revert Read/Write Enabled if you modified it
        foreach (var tex in processedTextures)
        {
            RevertReadWrite(tex);
        }

        AssetDatabase.Refresh();

        Debug.Log($"Materials combined successfully using shader '{templateMaterial.shader.name}'.");
    }

    private Material CollectMaterialsAndTextures(GameObject[] selectedObjects, List<Material> materials,
        List<TexturePropertySetting> activeSettings, List<Texture2D> processedTextures)
    {
        Material templateMaterial = null;
        Shader targetShader = null;

        foreach (GameObject obj in selectedObjects)
        {
            Renderer[] renderers = obj.GetComponentsInChildren<Renderer>(true); // Include LODs and inactive objects
            foreach (Renderer renderer in renderers)
            {
                Material[] mats = renderer.sharedMaterials;
                foreach (Material mat in mats)
                {
                    if (mat == null)
                    {
                        // Log a warning to notify about the null material
                        Debug.LogWarning($"Renderer on GameObject '{renderer.gameObject.name}' has a null material. Skipping.");
                        continue;
                    }

                    if (templateMaterial == null)
                    {
                        templateMaterial = mat;
                        targetShader = mat.shader;
                    }
                    else if (mat.shader != targetShader)
                    {
                        Debug.LogWarning($"Material '{mat.name}' uses shader '{mat.shader.name}', which doesn't match '{targetShader.name}'. Skipping.");
                        continue;
                    }

                    if (!materials.Contains(mat))
                    {
                        materials.Add(mat);

                        foreach (var setting in activeSettings)
                        {
                            string propertyName = setting.ResolvedPropertyName;
                            if (string.IsNullOrEmpty(propertyName))
                                continue;

                            Texture2D texture = null;

                            if (mat.HasProperty(propertyName))
                            {
                                texture = GetTextureFromMaterial(mat, propertyName);
                            }
                            else
                            {
                                Debug.LogWarning($"Material '{mat.name}' does not expose texture property '{propertyName}'. Using fallback for '{setting.label}'.");
                            }

                            if (texture != null)
                            {
                                EnsureTextureIsReadable(texture, processedTextures);
                                setting.textures[mat] = texture;
                            }
                            else
                            {
                                Texture2D fallback = EnsureFallbackTexture(setting);
                                if (fallback != null && !processedTextures.Contains(fallback))
                                {
                                    EnsureTextureIsReadable(fallback, processedTextures);
                                }
                                setting.textures[mat] = fallback;
                            }
                        }
                    }
                }
            }
        }

        return templateMaterial;
    }

    private Texture2D GetTextureFromMaterial(Material mat, string propertyName)
    {
        if (mat != null && mat.HasProperty(propertyName))
        {
            return mat.GetTexture(propertyName) as Texture2D;
        }
        return null;
    }

    private void EnsureTextureIsReadable(Texture2D texture, List<Texture2D> processedTextures)
    {
        if (texture == null)
            return;

        if (!IsReadWriteEnabled(texture))
        {
            EnableReadWrite(texture);
            processedTextures.Add(texture);
        }
    }

    private Rect[] CreateTextureAtlases(List<Material> materials, List<TexturePropertySetting> activeSettings,
        out Dictionary<TexturePropertySetting, Texture2D> atlases)
    {
        atlases = new Dictionary<TexturePropertySetting, Texture2D>();

        if (materials.Count == 0 || activeSettings.Count == 0)
        {
            return null;
        }

        const int atlasSize = 4096;
        const int padding = 0;

        TexturePropertySetting primarySetting = activeSettings[0];

        Texture2D primaryAtlas = new Texture2D(atlasSize, atlasSize);
        Texture2D[] primaryTextures = BuildTextureArray(materials, primarySetting);
        Rect[] rects = primaryAtlas.PackTextures(primaryTextures, padding, atlasSize, false);
        atlases[primarySetting] = primaryAtlas;

        for (int i = 1; i < activeSettings.Count; i++)
        {
            TexturePropertySetting setting = activeSettings[i];
            Texture2D[] textures = BuildTextureArray(materials, setting);
            Texture2D atlas = new Texture2D(atlasSize, atlasSize);
            atlas.PackTextures(textures, padding, atlasSize, false);
            atlases[setting] = atlas;
        }

        return rects;
    }

    private Texture2D[] BuildTextureArray(List<Material> materials, TexturePropertySetting setting)
    {
        Texture2D[] array = new Texture2D[materials.Count];
        for (int i = 0; i < materials.Count; i++)
        {
            Material mat = materials[i];
            if (!setting.textures.TryGetValue(mat, out Texture2D texture) || texture == null)
            {
                texture = EnsureFallbackTexture(setting);
            }
            array[i] = texture;
        }

        return array;
    }

    private Texture2D EnsureFallbackTexture(TexturePropertySetting setting)
    {
        if (setting.fallbackTexture != null)
            return setting.fallbackTexture;

        // Default to a neutral 2x2 texture if none provided
        Texture2D fallback = new Texture2D(2, 2, TextureFormat.RGBA32, false, !setting.sRGB);
        Color fillColor = Color.gray;

        if (setting.importerType == TextureImporterType.Default && setting.sRGB)
            fillColor = Color.white;
        else if (setting.importerType == TextureImporterType.NormalMap)
            fillColor = new Color(0.5f, 0.5f, 1f, 1f);
        else
            fillColor = Color.black;

        Color[] colors = Enumerable.Repeat(fillColor, 4).ToArray();
        fallback.SetPixels(colors);
        fallback.Apply();
        setting.fallbackTexture = fallback;
        return fallback;
    }

    private void SaveTextureAsAsset(Texture2D texture, string path, TexturePropertySetting setting)
    {
        byte[] bytes = texture.EncodeToPNG();
        File.WriteAllBytes(path, bytes);
        AssetDatabase.ImportAsset(path, ImportAssetOptions.ForceUpdate);

        // Set texture type and settings
        TextureImporter importer = AssetImporter.GetAtPath(path) as TextureImporter;
        if (importer != null)
        {
            importer.textureType = setting.importerType;
            importer.sRGBTexture = setting.sRGB;
            importer.mipmapEnabled = true;
            importer.isReadable = true;
            if (setting.importerType == TextureImporterType.NormalMap)
            {
                importer.convertToNormalmap = false;
            }
            importer.SaveAndReimport();
        }
    }

    private Material CreateCombinedMaterial(Material templateMaterial, Dictionary<TexturePropertySetting, string> atlasAssetPaths)
    {
        Material newMaterial = new Material(templateMaterial)
        {
            name = string.IsNullOrEmpty(templateMaterial.name) ? "CombinedMaterial" : templateMaterial.name + "_Combined"
        };

        foreach (var pair in atlasAssetPaths)
        {
            string propertyName = pair.Key.ResolvedPropertyName;
            if (string.IsNullOrEmpty(propertyName))
                continue;

            Texture2D atlasTexture = AssetDatabase.LoadAssetAtPath<Texture2D>(pair.Value);
            if (atlasTexture == null)
            {
                Debug.LogWarning($"Failed to load atlas texture at '{pair.Value}'.");
                continue;
            }

            if (newMaterial.HasProperty(propertyName))
            {
                newMaterial.SetTexture(propertyName, atlasTexture);
            }
            else
            {
                Debug.LogWarning($"Combined material shader does not have property '{propertyName}'.");
            }
        }

        return newMaterial;
    }

    private void ProcessGameObjects(GameObject[] selectedObjects, List<Material> materials, Rect[] rects, Material newMaterial)
    {
        foreach (GameObject obj in selectedObjects)
        {
            Renderer[] renderers = obj.GetComponentsInChildren<Renderer>(true); // Include LODs and inactive objects
            foreach (Renderer renderer in renderers)
            {
                MeshFilter meshFilter = renderer.GetComponent<MeshFilter>();
                if (meshFilter != null && renderer.sharedMaterials.Length > 0)
                {
                    Mesh mesh = Instantiate(meshFilter.sharedMesh);

                    Vector2[] originalUVs = mesh.uv;
                    Vector2[] uvs = new Vector2[originalUVs.Length];
                    Vector2[] originalUV2s = mesh.uv2.Length > 0 ? mesh.uv2 : originalUVs; // Use uv2 if available
                    Vector2[] uvs2 = new Vector2[originalUV2s.Length];

                    // Adjust UVs for each submesh
                    for (int subMeshIndex = 0; subMeshIndex < mesh.subMeshCount; subMeshIndex++)
                    {
                        Material mat = null;
                        if (subMeshIndex < renderer.sharedMaterials.Length)
                        {
                            mat = renderer.sharedMaterials[subMeshIndex];
                        }

                        if (mat == null)
                        {
                            // Log a warning about the missing material
                            Debug.LogWarning($"Renderer on GameObject '{renderer.gameObject.name}' is missing a material for submesh {subMeshIndex}. Skipping.");
                            continue;
                        }

                        int matIndex = materials.IndexOf(mat);
                        if (matIndex == -1)
                        {
                            Debug.LogWarning($"Material '{mat.name}' not found in materials list. Skipping.");
                            continue;
                        }

                        Rect rect = rects[matIndex];

                        int[] indices = mesh.GetTriangles(subMeshIndex);
                        for (int i = 0; i < indices.Length; i++)
                        {
                            int vertexIndex = indices[i];
                            uvs[vertexIndex].x = rect.x + originalUVs[vertexIndex].x * rect.width;
                            uvs[vertexIndex].y = rect.y + originalUVs[vertexIndex].y * rect.height;

                            // If you need to adjust uv2 or other UV sets, do it here
                        }
                    }

                    mesh.uv = uvs;

                    // Replace the mesh with the adjusted one
                    meshFilter.sharedMesh = mesh;

                    // Assign the new combined material
                    renderer.sharedMaterial = newMaterial;
                }
            }
        }
    }

    private bool IsReadWriteEnabled(Texture2D texture)
    {
        if (texture == null)
            return false;

        string assetPath = AssetDatabase.GetAssetPath(texture);
        if (string.IsNullOrEmpty(assetPath))
            return true;

        TextureImporter importer = AssetImporter.GetAtPath(assetPath) as TextureImporter;
        return importer != null && importer.isReadable;
    }

    private void EnableReadWrite(Texture2D texture)
    {
        if (texture == null)
            return;

        string assetPath = AssetDatabase.GetAssetPath(texture);
        if (string.IsNullOrEmpty(assetPath))
            return;

        TextureImporter importer = AssetImporter.GetAtPath(assetPath) as TextureImporter;

        if (importer != null && !importer.isReadable)
        {
            importer.isReadable = true;
            importer.SaveAndReimport();
            Debug.Log($"Enabled Read/Write on texture: {assetPath}");
        }
    }

    private void RevertReadWrite(Texture2D texture)
    {
        // Optionally revert the Read/Write setting to its original state
        // This requires storing the original state before changing it
        // For simplicity, this function is left empty
        // Implement as needed based on your workflow
    }
}