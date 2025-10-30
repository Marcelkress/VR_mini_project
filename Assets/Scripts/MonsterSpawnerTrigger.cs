using System;
using UnityEngine;

public class MonsterSpawnerTrigger : MonoBehaviour
{
    public int hordeSize, monsterIndex, spawnerPosIndex;
    public Transform spawnPoint;
    public HordeManager hordeManager;
    
    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
        {
            hordeManager.SpawnHorde(monsterIndex, hordeSize, spawnPoint.position, 1, 1);
        }
    }
}
