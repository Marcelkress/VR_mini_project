using Unity.VisualScripting;
using UnityEngine;

public interface IDamagable
{
    void TakeDamage(int damage);
    void Die();
}