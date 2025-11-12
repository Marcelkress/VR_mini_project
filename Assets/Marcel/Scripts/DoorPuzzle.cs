using System;
using UnityEngine;
using DG.Tweening;
using UnityEngine.XR.Interaction.Toolkit.Interactables;

public class DoorPuzzle : MonoBehaviour
{
    private Rigidbody doorBody;
    
    public Transform keySnapPos;
    public float keyFlyDuratio = 1f;
    
    
    public Vector3 unlockPushTorque;
    
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        doorBody = GetComponentInChildren<Rigidbody>();

        doorBody.freezeRotation = true;
        doorBody.isKinematic = true;
    }

    private void Unlock(GameObject key)
    {
        key.transform.parent = this.transform;
        key.transform.DOMove(keySnapPos.position, keyFlyDuratio, false);
        key.transform.DORotate(new Vector3(0, 0, 90), keyFlyDuratio);
        //key.GetComponent<Rigidbody>().isKinematic = true;

        doorBody.isKinematic = false;
        doorBody.freezeRotation = false;
        doorBody.AddTorque(unlockPushTorque, ForceMode.Impulse);
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("DoorKey"))
        {
            if (!other.GetComponent<XRGrabInteractable>().isSelected)
            {
                Unlock(other.gameObject);
            }
        }
    }
}
