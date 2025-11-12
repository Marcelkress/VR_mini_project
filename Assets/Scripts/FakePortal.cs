using System;
using UnityEngine;

public class FakePortal : MonoBehaviour
{
    public GameObject realRooms, fakeRooms;

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
            
            other.transform.SetParent(realRooms.transform);

            other.transform.localPosition = localposition;

            portalCounter++;

            if (portalCounter >= portalCounterTrigger)
            {
                SpawnNewObject();
            }
        }
    }

    void SpawnNewObject()
    {
        newObjects.SetActive(true);
    }
}
