using System;
using UnityEngine;
using UnityEngine.SceneManagement;

public class FakePortal : MonoBehaviour
{
    public GameObject realRooms, fakeRooms;
    public HordeManager HordeManager;
    public Transform monsterSpawnPoint;

    private Vector3 localposition;

    private int portalCounter;

    public GameObject bonFire, Key;

    public bool shouldSpawnMonsters = true;
    public bool reloadSceneAfterUse = false;

    private void OnTriggerEnter(Collider other)
    {
        if (reloadSceneAfterUse)
        {
            if (other.CompareTag("Player"))
            {
                other.transform.SetParent(fakeRooms.transform);

                localposition = other.transform.localPosition;
                HordeManager.CleanupDeadMonsters(); // Call to clear dead so you cant see them laying in the new room
                other.transform.SetParent(realRooms.transform);

                other.transform.localPosition = localposition;
        
                SceneManager.LoadScene(SceneManager.GetActiveScene().buildIndex);
            }

            return;
        }
        else if (other.CompareTag("Player"))
        {
            Debug.Log("Wal");

            other.transform.SetParent(fakeRooms.transform);

            localposition = other.transform.localPosition;
            HordeManager.CleanupDeadMonsters(); // Call to clear dead so you cant see them laying in the new room
            other.transform.SetParent(realRooms.transform);

            other.transform.localPosition = localposition;

            portalCounter++;

            StartCoroutine(HordeManager.SpawnHorde(1, 1, monsterSpawnPoint.position, 1, 1));

            if (portalCounter == 2)
            {
                bonFire.SetActive(true);
            }
            else if (portalCounter == 3)
            {
                Key.SetActive(true);
                bonFire.SetActive(false);
            }
        }
    }
    
}
