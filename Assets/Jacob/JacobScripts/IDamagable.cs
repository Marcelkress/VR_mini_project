using Unity.VisualScripting;
using UnityEngine;

public interface IDamagable
{
    void OnTriggerEnter(Collider other);

    void OnTriggerExit(Collider other);
}