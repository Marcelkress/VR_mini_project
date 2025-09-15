using UnityEngine;
using System.Collections.Generic;

public class HordeManager : MonoBehaviour
{
    [Header("Horde Settings")]
    public GameObject monsterPrefab;
    public Transform player; // Assign your player here
    
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
        
        SpawnHorde(20, spawnPoints[0].position, 15f, 20f);
    }
    
    // Kan kaldes fra et script hvor vi vil spawn en horde af monstre, fx når spilleren når et bestemt område
    public void SpawnHorde(int hordeSize, Vector3 spawnpoint, float spawnRadius, float spawnDistance)
    {
        if (monsterPrefab == null || player == null) return;

        for (int i = 0; i < hordeSize; i++)
        {
            Vector3 spawnPosition = GetSpawnPosition(i, spawnRadius, spawnDistance, hordeSize);

            GameObject monster = Instantiate(monsterPrefab, spawnPosition, Quaternion.identity);
            MonsterTest monsterScript = monster.GetComponent<MonsterTest>();

            if (monsterScript != null)
            {
                // Assign the player as target
                monsterScript.SetTarget(player);
                spawnedMonsters.Add(monsterScript);
            }
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
    
    // Change target for all monsters (useful if player changes)
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
}
