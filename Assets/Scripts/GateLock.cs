using DG.Tweening;
using UnityEngine;
using UnityEngine.XR.Interaction.Toolkit.Interactables;

public class GateLock : MonoBehaviour
{
    public GameObject leftLock, rightLock;
    public float keyFloatDuration;
    
    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("GateKeyLeft"))
        {
            if (!other.GetComponent<XRGrabInteractable>().isSelected)
            {
                Unlock(other.gameObject, true);
            }
        }
        else if (other.CompareTag("GateKeyRight"))
        {
            if (!other.GetComponent<XRGrabInteractable>().isSelected)
            {
                Unlock(other.gameObject, false);
            }
        }
    }

    private void Unlock(GameObject key, bool left)
    {
        if (left)
        {
            key.transform.parent = this.transform;
            key.transform.DOMove(leftLock.transform.position, keyFloatDuration, false);
            //key.transform.DORotate(new Vector3(0, 0, 90), keyFloatDuration);
        }
        else
        {
            key.transform.parent = this.transform;
            key.transform.DOMove(rightLock.transform.position, keyFloatDuration, false);
            //key.transform.DORotate(new Vector3(0, 0, 90), keyFloatDuration);
        }
    }


}
