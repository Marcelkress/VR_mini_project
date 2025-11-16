using System.Numerics;
using FMODUnity;
using UnityEngine;
using UnityEngine.Events;

public class PotionDrink : MonoBehaviour
{
    [Header("Potion Settings")]
    [Tooltip("Amount of health restored when drinking")]
    public int healAmount = 30;
    
    [Tooltip("Height above player's head position to trigger drink")]
    public float drinkHeight = 0.3f;
    
    [Tooltip("Reference to the player's head/camera transform")]
    public Transform playerHead;

    [Tooltip("Reference to the player health system")]
    public PlayerHealthSystem healthSystem;
    
    public GameObject EmptyBottlePrefab;

    private bool hasBeenDrunk = false;

    public UnityEvent OnPotionDrunk;
    
    public StudioEventEmitter potionDrinkSound;
    


    void Update()
    {
        if (hasBeenDrunk || playerHead == null || healthSystem == null)
            return;

        // Check if potion is elevated above player's head
        if (transform.position.y >= playerHead.position.y + drinkHeight)
        {
            DrinkPotion();
            potionDrinkSound.Play();
        }
    }

    private void DrinkPotion()
    {
        hasBeenDrunk = true;
        Debug.Log("Potion consumed! Healing player by " + healAmount);
        
        OnPotionDrunk.Invoke();
        healthSystem.Heal(healAmount);
        
        EmptyBottlePrefab.SetActive(false);     

    }
}
