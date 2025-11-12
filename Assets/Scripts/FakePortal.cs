using System;
using UnityEngine;

public class FakePortal : MonoBehaviour
{
    public GameObject realRooms, fakeRooms;
    public HordeManager HordeManager;
    public Transform monsterSpawnPoint;

    private Vector3 localposition;

    private int portalCounter;
    public int portalCounterTrigger;

    public GameObject newObjects;

    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
        {
            Debug.Log("Wal");

            other.transform.SetParent(fakeRooms.transform);

            localposition = other.transform.localPosition;
            HordeManager.CleanupDeadMonsters(); // Call to clear dead so you cant see them laying in the new room
            other.transform.SetParent(realRooms.transform);

            other.transform.localPosition = localposition;

            portalCounter++;

            StartCoroutine(HordeManager.SpawnHorde(1, 1, monsterSpawnPoint.position, 1, 1));

            if (portalCounter >= portalCounterTrigger)
            {
                SpawnNewObject();
            }
        }
    }
    
    private void ClearDeadMonsters()
    {
        
    }

    void SpawnNewObject()
    {
        newObjects.SetActive(true);
    }
}
