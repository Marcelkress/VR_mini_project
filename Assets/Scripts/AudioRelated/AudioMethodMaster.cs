using System;
using FMODUnity;
using UnityEngine;

public class AudioMethodMaster : MonoBehaviour
{
    
    public StudioEventEmitter MonsterAttack;
    public StudioEventEmitter Weponwoosh;
   
    
    public void monsterAttack()
    {
        if (MonsterAttack != null)
        {
            MonsterAttack.Play();
        }
       
        
    }
    public void weponWoosh()
    {
        if (Weponwoosh != null)
        {
            Weponwoosh.Play();
        }
        
    }
    
    
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
