using DG.Tweening;
using FMODUnity;
using UnityEngine;
using UnityEngine.XR.Interaction.Toolkit.Interactables;

public class GateLock : MonoBehaviour
{
    public GameObject leftLock, rightLock, rightGate, leftGate;
    public Transform leftKeyPos, rightKeyPos;
    public Vector3 rotation;
    public Animator leftLineAnim, rightLineAnim;
    public StudioEventEmitter LeftKeyunlockSound;
    public StudioEventEmitter RightKeyunlockSound;
    
    public float keyFloatDuration;

    private bool leftOpen, rightOpen;
    
    private void Start()
    {
        rightGate.GetComponent<HingeJoint>().useMotor = false;
        leftGate.GetComponent<HingeJoint>().useMotor = false;

        rightGate.GetComponent<Rigidbody>().constraints = RigidbodyConstraints.FreezeAll;
        leftGate.GetComponent<Rigidbody>().constraints = RigidbodyConstraints.FreezeAll;
    }
    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("GateKeyLeft"))
        {
            if (!other.GetComponent<XRGrabInteractable>().isSelected)
            {
                Unlock(other.gameObject, true);
                leftOpen = true;
                LeftKeyunlockSound.Play();
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
                RightKeyunlockSound.Play();
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

            leftGate.GetComponent<Rigidbody>().constraints = RigidbodyConstraints.None;
            rightGate.GetComponent<Rigidbody>().constraints = RigidbodyConstraints.None;

            // Enable motors and set target velocity to open the gates
            JointMotor leftMotor = leftGate.GetComponent<HingeJoint>().motor;
            leftMotor.targetVelocity = 90f; 
            leftMotor.force = 10f; 
            leftGate.GetComponent<HingeJoint>().motor = leftMotor;
            leftGate.GetComponent<HingeJoint>().useMotor = true;

            JointMotor rightMotor = rightGate.GetComponent<HingeJoint>().motor;
            rightMotor.targetVelocity = 90f;
            rightMotor.force = 10f;
            rightGate.GetComponent<HingeJoint>().motor = rightMotor;
            rightGate.GetComponent<HingeJoint>().useMotor = true;



        }
    }


}
