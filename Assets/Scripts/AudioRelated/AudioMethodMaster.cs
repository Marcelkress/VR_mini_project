using System;
using FMODUnity;
using UnityEngine;

public class AudioMethodMaster : MonoBehaviour
{
    
    public StudioEventEmitter MonsterAttack;
    public StudioEventEmitter HandsClick;
    public StudioEventEmitter VHSTapeInsert;
    public StudioEventEmitter whiteNoise;
    public StudioEventEmitter VHSgrab;
    
    public void monsterAttack()
    {
        if (MonsterAttack != null)
        {
            MonsterAttack.Play();
        }
       
        
    }

    public void testaudio2()
    {
        if (HandsClick != null)
        {
            HandsClick.Play();
        }
        
    }
    
    
    public void VHSAudio()
    {
        if (VHSTapeInsert != null)
        {
            VHSTapeInsert.Play();
        }
        
    }
    
    public void WhiteNoiseAudio()
    {
        if (whiteNoise != null)
        {
            whiteNoise.Play();
            
            
        }
        
    }
    
    public void VhsGrabAudio()
    {
        if (VHSgrab != null)
        {
            VHSgrab.Play();
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
