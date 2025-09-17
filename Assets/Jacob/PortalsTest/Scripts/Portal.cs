using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(BoxCollider))]
public class Portal : MonoBehaviour
{
    [field: SerializeField]
    public Portal OtherPortal { get; private set; }

    [SerializeField]
    private Renderer outlineRenderer;

    [field: SerializeField]
    public Color PortalColour { get; private set; }

    [SerializeField]
    private LayerMask placementMask;

    [SerializeField]
    private Transform testTransform;

    private List<PortalableObject> portalObjects = new List<PortalableObject>();
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

        // If manually placed in the scene and wallCollider isn't assigned, try to auto-detect by raycasting backwards.
        if (wallCollider == null)
        {
            RaycastHit hit;
            if (Physics.Raycast(transform.position + transform.forward * 0.05f, -transform.forward, out hit, 0.2f))
            {
                wallCollider = hit.collider;
            }
        }
    }

    private void Start()
    {
        if (outlineRenderer != null && outlineRenderer.material != null)
        {
            outlineRenderer.material.SetColor("_OutlineColour", PortalColour);
        }
        
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

        for (int i = 0; i < portalObjects.Count; ++i)
        {
            Vector3 objPos = transform.InverseTransformPoint(portalObjects[i].transform.position);

            if (objPos.z > 0.0f)
            {
                portalObjects[i].Warp();
            }
        }
    }

    private void OnTriggerEnter(Collider other)
    {
        var obj = other.GetComponent<PortalableObject>();
        if (obj != null)
        {
            portalObjects.Add(obj);
            obj.SetIsInPortal(this, OtherPortal, wallCollider);
        }
    }

    private void OnTriggerExit(Collider other)
    {
        var obj = other.GetComponent<PortalableObject>();

        if(portalObjects.Contains(obj))
        {
            portalObjects.Remove(obj);
            obj.ExitPortal(wallCollider);
        }
    }

}
