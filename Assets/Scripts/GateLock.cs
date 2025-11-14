using DG.Tweening;
using UnityEngine;
using UnityEngine.XR.Interaction.Toolkit.Interactables;

public class GateLock : MonoBehaviour
{
    public GameObject leftLock, rightLock;
    public Transform leftKeyPos, rightKeyPos;
    public Vector3 rotation;
    public Animator leftLineAnim, rightLineAnim;
    
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
                leftLineAnim.SetTrigger("Lightup");
            }

            other.GetComponent<XRGrabInteractable>().enabled = false;
        }
        else if (other.CompareTag("GateKeyRight"))
        {
            if (!other.GetComponent<XRGrabInteractable>().isSelected)
            {
                Unlock(other.gameObject, false);
                rightOpen = true;
                rightLineAnim.SetTrigger("Lightup");
            }
            
            other.GetComponent<XRGrabInteractable>().enabled = false;
        }

        if (rightOpen && leftOpen)
        {
            
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
