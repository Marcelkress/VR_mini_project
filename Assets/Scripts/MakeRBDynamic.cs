using UnityEngine;

public class MakeRBDynamic : MonoBehaviour
{
    public void PickUp()
    {
        GetComponent<Rigidbody>().isKinematic = false;
    }
}
