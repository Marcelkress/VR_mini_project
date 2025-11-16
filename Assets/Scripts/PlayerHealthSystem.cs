using System.Collections;
using DG.Tweening;
using UnityEngine;
using UnityEngine.Events;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using UnityEngine.XR.Interaction.Toolkit.Samples.StarterAssets;

public class PlayerHealthSystem : MonoBehaviour
{
    public int maxHealth = 100;
    public int currentHealth;
    private bool lowHealthTriggered = false;

    public RawImage dieImage;
    public float fadeTime;
    public float reloadSceneTime;
    
    public BloodVignette bloodVignette;
    private HordeManager hordemanager;

    public UnityEvent LowHealthEvent, TakeDamageEvent, DeathEvent;

    void Start()
    {
        currentHealth = maxHealth;
        hordemanager = FindObjectOfType<HordeManager>();
    }

    void Update()
    {
        DynamicEvents();
    }

    private void DynamicEvents()
    {
        // Check if health is low (below 30%) and trigger event once
        if (currentHealth <= maxHealth * 0.3f && !lowHealthTriggered)
        {
            LowHealthEvent.Invoke();
            lowHealthTriggered = true;
        }

        // Reset flag when health is restored above low threshold
        if (currentHealth > maxHealth * 0.3f && lowHealthTriggered)
        {
            lowHealthTriggered = false;
        }
    }
    
    public void Heal(int healAmount)
    {
        if (healAmount <= 0 || currentHealth == maxHealth) return;

        currentHealth += healAmount;
        bloodVignette.UpdateVignetteParameters();
        if (currentHealth > maxHealth)
        {
            currentHealth = maxHealth;
        }
    }

    public void TakeDamage(int damage)
    {
        currentHealth -= damage;
        Debug.Log("Player took " + damage + " damage. Current health: " + currentHealth);
        TakeDamageEvent.Invoke();
        bloodVignette.UpdateVignetteParameters();
        if (currentHealth <= 0)
        {
            Die();
        }


    }

    public float GetHealthPercentage()
    {
        return (float)currentHealth / maxHealth;
    }

    
    private void stopmonstersounds()
    {
        if (hordemanager != null)
        {
            hordemanager.StopAllMonsterSounds();
        }
    }
    
    private void Die()
    
    
    {
        Debug.Log("Player has died.");
        DeathEvent.Invoke();
        dieImage.DOFade(1, fadeTime);
        

        GetComponentInChildren<DynamicMoveProvider>().moveSpeed = 0;

        StartCoroutine(ReloadSceneWait());
    }

    private IEnumerator ReloadSceneWait()
    {
        yield return new WaitForSeconds(reloadSceneTime);
        var scene = SceneManager.GetActiveScene();
        SceneManager.LoadScene(scene.buildIndex);
        stopmonstersounds();

        yield return null;
    }

}
