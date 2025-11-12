using DG.Tweening;
using UnityEngine;
using UnityEngine.XR.Interaction.Toolkit.Interactables;

public class GateLock : MonoBehaviour
{
    public GameObject leftLock, rightLock;
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

    private void Unlock(GameObject key, bool left)
    {
        if (left)
        {
            key.transform.parent = this.transform;
            key.transform.DOMove(leftLock.transform.position, keyFloatDuration, false);
            key.transform.DORotate(new Vector3(-90, 0, 0), keyFloatDuration);
        }
        else
        {
            key.transform.parent = this.transform;
            key.transform.DOMove(rightLock.transform.position, keyFloatDuration, false);
            key.transform.DORotate(new Vector3(-90, 0, 0), keyFloatDuration);
        }
        
        key.GetComponent<Rigidbody>().isKinematic = true;

        if (leftOpen && rightOpen)
        {
            // WIN!!
        }
    }


}
