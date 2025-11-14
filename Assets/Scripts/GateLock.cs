using DG.Tweening;
using UnityEngine;
using UnityEngine.XR.Interaction.Toolkit.Interactables;

public class GateLock : MonoBehaviour
{
    public GameObject leftLock, rightLock;
    public Transform leftKeyPos, rightKeyPos;
    public Vector3 rotation;
    public Light leftLight, rightLight;
    public float lightTargetIntensity, ligthFadeTime = 2f;
    
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
                leftLight.DOIntensity(lightTargetIntensity, ligthFadeTime);
            }

            other.GetComponent<XRGrabInteractable>().enabled = false;
        }
        else if (other.CompareTag("GateKeyRight"))
        {
            if (!other.GetComponent<XRGrabInteractable>().isSelected)
            {
                Unlock(other.gameObject, false);
                rightOpen = true;
                rightLight.DOIntensity(lightTargetIntensity, ligthFadeTime);
            }
            
            other.GetComponent<XRGrabInteractable>().enabled = false;
        }
        
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
