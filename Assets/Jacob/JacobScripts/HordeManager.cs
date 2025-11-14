using System.Collections;
using UnityEngine;
using System.Collections.Generic;

public class HordeManager : MonoBehaviour
{
    [Header("Horde Settings")]
    public GameObject[] monsterPrefab; // Array of monster prefabs to spawn
    public Transform player; // Assign your player here
    public int hordeSpawnSize = 20; // Max number of monsters in a horde
    public float spawnInterval = 1;
    
    [Header("Spawn Points (Optional)")]
    public Transform[] spawnPoints; // If you want specific spawn locations

    
    private List<MonsterTest> spawnedMonsters = new List<MonsterTest>();

    void Start()
    {
        if (player == null)
        {
            // Try to find player by tag if not assigned
            GameObject playerObj = GameObject.FindGameObjectWithTag("Player");
            if (playerObj != null)
                player = playerObj.transform;
            else
                Debug.LogError("Player not found! Please assign the player transform.");
        }
        
        // Horde test:
        // SpawnHorde(monsterPrefab, hordeSpawnSize, spawnPoints[0].position, spawnRadius: 15f, spawnDistance: 20f);
    }
    
    public void HordeMonsterPopulator(int monsterIndex)
    {
        StartCoroutine(SpawnHorde(monsterIndex, hordeSpawnSize, player.position, spawnRadius: 1f, spawnDistance: 1f));
    }
    
    // Kan kaldes fra et script hvor vi vil spawn en horde af monstre, fx når spilleren når et bestemt område
    public IEnumerator SpawnHorde(int monsterIndex, int hordeSize, Vector3 spawnpoint, float spawnRadius, float spawnDistance)
    {
        if (monsterPrefab == null || player == null)
        {
            yield break;
        }

        for (int i = 0; i < hordeSize; i++)
        {
            //Vector3 spawnPosition = GetSpawnPosition(i, spawnRadius, spawnDistance, hordeSize);
            
            GameObject monster = Instantiate(monsterPrefab[monsterIndex], spawnpoint, Quaternion.identity);
            MonsterTest monsterScript = monster.GetComponent<MonsterTest>();

            if (monsterScript != null)
            {
                // Assign the player as target
                monsterScript.SetTarget(player);
                spawnedMonsters.Add(monsterScript);
            }

            yield return new WaitForSeconds(spawnInterval);
        }

        Debug.Log($"Spawned {hordeSize} monsters targeting player!");
    }
    
    // Finder spawn position for et monster
    private Vector3 GetSpawnPosition(int index, float spawnRadius, float spawnDistance, int hordeSize)
    {
        // If specific spawn points are provided, use them
        if (spawnPoints != null && spawnPoints.Length > 0)
        {
            return spawnPoints[index % spawnPoints.Length].position;
        }

        // Otherwise, spawn in a circle around the player
        float angle = (360f / hordeSize) * index;
        Vector3 direction = Quaternion.Euler(0, angle, 0) * Vector3.forward;
        Vector3 spawnPos = player.position + direction * spawnDistance;

        // Add some random variation
        spawnPos += new Vector3(
            Random.Range(-spawnRadius * 0.5f, spawnRadius * 0.5f),
            0,
            Random.Range(-spawnRadius * 0.5f, spawnRadius * 0.5f)
        );

        return spawnPos;
    }
    
    // Remove dead monsters from tracking
    public void CleanupDeadMonsters()
    {
        spawnedMonsters.RemoveAll(monster => monster == null);
    }
    
    // Get count of remaining alive monsters
    public int GetAliveMonsterCount()
    {
        CleanupDeadMonsters();
        return spawnedMonsters.Count;
    }
    
    // ændrer target for alle monstre i horde til newTarget hvis vi får brug for det
    public void UpdateAllTargets(Transform newTarget)
    {
        player = newTarget;
        foreach (MonsterTest monster in spawnedMonsters)
        {
            if (monster != null)
            {
                monster.SetTarget(newTarget);
            }
        }
    }

    public void SpawnHordeForKey()
    {
        StartCoroutine(specialHordeCoroutine());
    }

    private IEnumerator specialHordeCoroutine()
    {
        yield return SpawnHorde(0, 10, spawnPoints[0].transform.position, spawnRadius: 5f, spawnDistance: 10f);
        yield return new WaitForSeconds(3f);
        yield return SpawnHorde(1, 5, spawnPoints[2].transform.position, spawnRadius: 5f, spawnDistance: 10f);
        yield return new WaitForSeconds(3f);
        yield return SpawnHorde(1, 3, spawnPoints[0].transform.position, spawnRadius: 5f, spawnDistance: 10f);
    }
}
