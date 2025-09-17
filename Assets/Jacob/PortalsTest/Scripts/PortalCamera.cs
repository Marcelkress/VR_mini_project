using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;
using RenderPipeline = UnityEngine.Rendering.RenderPipelineManager;

public class PortalCamera : MonoBehaviour
{
    [SerializeField]
    private Portal[] portals = new Portal[2];

    [SerializeField]
    private Camera portalCamera;

    [SerializeField]
    private int iterations = 7;

    private RenderTexture tempTexture1;
    private RenderTexture tempTexture2;

    private Camera mainCamera;

    private void Awake()
    {
        // Prefer a camera on this object, otherwise fallback to children or main camera
        mainCamera = GetComponent<Camera>();
        if (mainCamera == null)
        {
            mainCamera = GetComponentInChildren<Camera>();
        }
        if (mainCamera == null)
        {
            mainCamera = Camera.main;
        }
        
        if (mainCamera == null)
        {
            Debug.LogError("PortalCamera: No main camera found! Make sure Camera.main is properly set or assign a camera to this object.");
            return;
        }

        // Use main camera resolution for better compatibility, fallback to screen size if needed
        int width = mainCamera.pixelWidth > 0 ? mainCamera.pixelWidth : Screen.width;
        int height = mainCamera.pixelHeight > 0 ? mainCamera.pixelHeight : Screen.height;
        
        // Ensure minimum resolution
        width = Mathf.Max(width, 512);
        height = Mathf.Max(height, 512);

        tempTexture1 = new RenderTexture(width, height, 24, RenderTextureFormat.ARGB32);
        tempTexture2 = new RenderTexture(width, height, 24, RenderTextureFormat.ARGB32);
        
        // Create the render textures
        tempTexture1.Create();
        tempTexture2.Create();
    }

    private void Start()
    {
        // Validate setup before proceeding
        if (tempTexture1 == null || tempTexture2 == null)
        {
            Debug.LogError("PortalCamera: Render textures not created properly!");
            return;
        }
        
        if (portals == null || portals.Length < 2)
        {
            Debug.LogError("PortalCamera: Please assign both portals in the inspector!");
            return;
        }
        
        if (portalCamera == null)
        {
            Debug.LogError("PortalCamera: Please assign the portal camera in the inspector!");
            return;
        }

        // Assign to both legacy and URP/HDRP base map properties
        if (portals[0] != null && portals[0].Renderer != null)
        {
            var mat0 = portals[0].Renderer.material;
            if (mat0 != null)
            {
                if (mat0.HasProperty("_MainTex")) mat0.SetTexture("_MainTex", tempTexture1);
                if (mat0.HasProperty("_BaseMap")) mat0.SetTexture("_BaseMap", tempTexture1);
                Debug.Log($"PortalCamera: Assigned tempTexture1 to portal {portals[0].name}");
            }
            else
            {
                Debug.LogWarning($"PortalCamera: Portal {portals[0].name} has no material!");
            }
        }
        
        if (portals[1] != null && portals[1].Renderer != null)
        {
            var mat1 = portals[1].Renderer.material;
            if (mat1 != null)
            {
                if (mat1.HasProperty("_MainTex")) mat1.SetTexture("_MainTex", tempTexture2);
                if (mat1.HasProperty("_BaseMap")) mat1.SetTexture("_BaseMap", tempTexture2);
                Debug.Log($"PortalCamera: Assigned tempTexture2 to portal {portals[1].name}");
            }
            else
            {
                Debug.LogWarning($"PortalCamera: Portal {portals[1].name} has no material!");
            }
        }
    }

    private void OnEnable()
    {
        RenderPipeline.beginCameraRendering += UpdateCamera;
    }

    private void OnDisable()
    {
        RenderPipeline.beginCameraRendering -= UpdateCamera;
    }

    void UpdateCamera(ScriptableRenderContext SRC, Camera camera)
    {
        // Only update for the main camera, not for portal cameras or other cameras
        if (camera != mainCamera)
        {
            return;
        }
        
        if (portals == null || portals.Length < 2 || portals[0] == null || portals[1] == null)
        {
            return;
        }
        if (!portals[0].IsPlaced || !portals[1].IsPlaced)
        {
            return;
        }
        
        if (portalCamera == null)
        {
            Debug.LogError("Portal Camera is not assigned!");
            return;
        }

        if (portals[0].Renderer != null && portals[0].Renderer.isVisible)
        {
            portalCamera.targetTexture = tempTexture1;
            for (int i = iterations - 1; i >= 0; --i)
            {
                RenderCamera(portals[0], portals[1], i, SRC);
            }
        }

        if (portals[1].Renderer != null && portals[1].Renderer.isVisible)
        {
            portalCamera.targetTexture = tempTexture2;
            for (int i = iterations - 1; i >= 0; --i)
            {
                RenderCamera(portals[1], portals[0], i, SRC);
            }
        }
    }

    private void RenderCamera(Portal inPortal, Portal outPortal, int iterationID, ScriptableRenderContext SRC)
    {
        Transform inTransform = inPortal.transform;
        Transform outTransform = outPortal.transform;

        Transform cameraTransform = portalCamera.transform;
        cameraTransform.position = transform.position;
        cameraTransform.rotation = transform.rotation;

        for(int i = 0; i <= iterationID; ++i)
        {
            // Position the camera behind the other portal.
            Vector3 relativePos = inTransform.InverseTransformPoint(cameraTransform.position);
            relativePos = Quaternion.Euler(0.0f, 180.0f, 0.0f) * relativePos;
            cameraTransform.position = outTransform.TransformPoint(relativePos);

            // Rotate the camera to look through the other portal.
            Quaternion relativeRot = Quaternion.Inverse(inTransform.rotation) * cameraTransform.rotation;
            relativeRot = Quaternion.Euler(0.0f, 180.0f, 0.0f) * relativeRot;
            cameraTransform.rotation = outTransform.rotation * relativeRot;
        }

        // Set the camera's oblique view frustum.
        Plane p = new Plane(-outTransform.forward, outTransform.position);
        Vector4 clipPlaneWorldSpace = new Vector4(p.normal.x, p.normal.y, p.normal.z, p.distance);
        Vector4 clipPlaneCameraSpace =
            Matrix4x4.Transpose(Matrix4x4.Inverse(portalCamera.worldToCameraMatrix)) * clipPlaneWorldSpace;

        var newMatrix = mainCamera.CalculateObliqueMatrix(clipPlaneCameraSpace);
        portalCamera.projectionMatrix = newMatrix;

        // Render the camera to its render target using direct camera render.
        portalCamera.Render();
    }
}
