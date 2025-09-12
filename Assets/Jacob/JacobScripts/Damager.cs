using UnityEngine;

[RequireComponent(typeof(Collider)), RequireComponent(typeof(Rigidbody))]
public class Damager : MonoBehaviour
{
    public int damageAmount = 10;

    void Start()
    {
        GetComponent<Collider>().isTrigger = false;
        GetComponent<Rigidbody>().useGravity = false;
    }


}
