using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(BoxCollider))]
public class Portal : MonoBehaviour
{
    [field: SerializeField]
    public Portal OtherPortal { get; private set; }

    [field: SerializeField]
    public Color PortalColour { get; private set; }

    [SerializeField]
    private LayerMask placementMask;

    [SerializeField]
    private Transform testTransform;

    private List<PortalableObject> portalObjects = new List<PortalableObject>();
    // Track previous Z in portal-local space for each tracked object so we can detect crossing the portal plane.
    private Dictionary<PortalableObject, float> previousLocalZ = new Dictionary<PortalableObject, float>();
    public bool IsPlaced { get; private set; } = false;
    [SerializeField]
    [Tooltip("Collider of the wall/surface this portal is mounted on. Optional for manual placement; will be set automatically on successful PlacePortal().")]
    private Collider wallCollider;

    // Components.
    public Renderer Renderer { get; private set; }
    private new BoxCollider collider;

    private void Awake()
    {
        collider = GetComponent<BoxCollider>();
        // Try MeshRenderer first (typical for portal visual quad); fallback to any Renderer.
        Renderer = GetComponent<MeshRenderer>();
        if (Renderer == null)
        {
            Renderer = GetComponent<Renderer>();
        }
    }

    private void Start()
    {
        
        // Validation for manual placement
        if (OtherPortal == null)
        {
            Debug.LogError($"Portal '{gameObject.name}' does not have an OtherPortal reference assigned!");
            return;
        }
        
        // Ensure the other portal also references this one
        if (OtherPortal.OtherPortal != this)
        {
            Debug.LogWarning($"Portal '{gameObject.name}' and '{OtherPortal.gameObject.name}' don't have mutual references. This may cause issues.");
        }

        gameObject.SetActive(true);
        IsPlaced = true;
    }

    private void Update()
    {
        // Enable portal renderer only when both portals are properly set up
        Renderer.enabled = OtherPortal != null && OtherPortal.IsPlaced && IsPlaced;

        // Iterate over a copy since we may modify the collection during iteration (warp moves objects to the other portal).
        for (int i = portalObjects.Count - 1; i >= 0; --i)
        {
            var obj = portalObjects[i];

            if (obj == null)
            {
                portalObjects.RemoveAt(i);
                previousLocalZ.Remove(obj);
                continue;
            }

            float currentZ = transform.InverseTransformPoint(obj.transform.position).z;
            float prevZ = 0.0f;
            previousLocalZ.TryGetValue(obj, out prevZ);

            // Skip if recently warped to prevent double triggers.
            if (Time.time - obj.GetLastWarpTime() < 0.1f)
            {
                // Update previous Z for next frame.
                previousLocalZ[obj] = currentZ;
                continue;
            }

            // Detect crossing the portal plane in either direction (sign change across Z=0).
            if ((prevZ > 0.0f && currentZ <= 0.0f) || (prevZ <= 0.0f && currentZ > 0.0f))
            {
                // Warp the object through this portal to the other one.
                obj.Warp();

                // Remove from this portal's tracking state.
                portalObjects.RemoveAt(i);
                previousLocalZ.Remove(obj);

                // Ensure the object is registered with the destination portal so it can travel back.
                if (OtherPortal != null)
                {
                    OtherPortal.RegisterObjectFromWarp(obj);
                }

                continue;
            }

            // Update previous Z for next frame.
            previousLocalZ[obj] = currentZ;
        }
    }

    private void OnTriggerEnter(Collider other)
    {
        var obj = other.GetComponent<PortalableObject>();
        if (obj != null)
        {
            portalObjects.Add(obj);
            // Initialize previous local Z for this object so we can detect crossings.
            previousLocalZ[obj] = transform.InverseTransformPoint(obj.transform.position).z;
            obj.SetIsInPortal(this, OtherPortal, wallCollider);
        }
    }

    private void OnTriggerExit(Collider other)
    {
        var obj = other.GetComponent<PortalableObject>();

        if(portalObjects.Contains(obj))
        {
            portalObjects.Remove(obj);
            previousLocalZ.Remove(obj);
            obj.ExitPortal(wallCollider);
        }
    }

    /// <summary>
    /// Called when an object was just warped into this portal's space by its paired portal.
    /// Registers tracking state and ensures the object's portal collision/clone state is set.
    /// </summary>
    /// <param name="obj">The object that was warped into this portal.</param>
    public void RegisterObjectFromWarp(PortalableObject obj)
    {
        if (obj == null)
            return;

        if (!portalObjects.Contains(obj))
        {
            portalObjects.Add(obj);
        }

        previousLocalZ[obj] = transform.InverseTransformPoint(obj.transform.position).z;

        // Ensure the object has its portal state configured for this portal pair.
        obj.SetIsInPortal(this, OtherPortal, wallCollider);
    }

}
