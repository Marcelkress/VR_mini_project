using DG.Tweening;
using UnityEngine;
using UnityEngine.XR.Interaction.Toolkit.Interactables;

public class GateLock : MonoBehaviour
{
    public GameObject leftLock, rightLock;
    public Transform leftKeyPos, rightKeyPos;
    public Vector3 rotation;
    public float emissionIntensity = 1f;
    
    public float keyFloatDuration;

    private bool leftOpen, rightOpen;
    
    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("GateKeyLeft"))
        {
            if (!other.GetComponent<XRGrabInteractable>().isSelected)
            {
                Unlock(other.gameObject, true);
                leftOpen = true;
                
                
                EnableEmission(other.GetComponent<MeshRenderer>(), Color.red, emissionIntensity);
            }
        }
        else if (other.CompareTag("GateKeyRight"))
        {
            if (!other.GetComponent<XRGrabInteractable>().isSelected)
            {
                Unlock(other.gameObject, false);
                rightOpen = true;
            }
        }
    }
    
    private void EnableEmission(Renderer renderer, Color color, float intensity = 1f)
    {
        if (renderer == null) return;

        var mat = renderer.material; // instance, not shared
        var emissive = color * intensity;

        if (mat.HasProperty("_EmissionColor"))
            mat.SetColor("_EmissionColor", emissive);
        else if (mat.HasProperty("_EmissiveColor"))
            mat.SetColor("_EmissiveColor", emissive);
        
        mat.EnableKeyword("_EMISSION");
        mat.globalIlluminationFlags = MaterialGlobalIlluminationFlags.RealtimeEmissive;
        DynamicGI.SetEmissive(renderer, emissive);
    }

    private void Unlock(GameObject key, bool left)
    {
        if (left)
        {
            key.transform.parent = this.transform;
            key.transform.DOMove(leftKeyPos.position, keyFloatDuration, false);
            key.transform.DORotate(rotation, keyFloatDuration);
        }
        else
        {
            key.transform.parent = this.transform;
            key.transform.DOMove(rightKeyPos.position, keyFloatDuration, false);
            key.transform.DORotate(rotation, keyFloatDuration);
        }
        
        key.GetComponent<Rigidbody>().isKinematic = true;

        if (leftOpen && rightOpen)
        {
            // WIN!!
        }
    }


}
