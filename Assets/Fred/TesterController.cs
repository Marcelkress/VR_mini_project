using UnityEngine;

public class TesterController : MonoBehaviour
{
    public float speed = 5f;
    public Rigidbody rb;

    void Update()
    {
        //rb.MovePosition(rb.position + Vector3.forward * speed * Time.fixedDeltaTime);
        transform.position += Vector3.forward * speed * Time.deltaTime;
    }
}
