using System;
using UnityEngine;
using FMODUnity;

public class SkeletonAudio : MonoBehaviour
{
    //public void PlaySkeletonAttack()
    //{
    //    RuntimeManager.PlayOneShot(FmodEvents.Instance.SkeletonAttack, transform.position);
   // }

    public void PlaySkeletonDamage()
    {
        RuntimeManager.PlayOneShot(FmodEvents.Instance.SkeletonDamage, transform.position);
    }

    //public void PlaySkeletonDeath()
   // {
    //    RuntimeManager.PlayOneShot(FmodEvents.Instance.SkeletonDeath, transform.position);
    //}

    // LOOPING BREATH EXAMPLE
    private FMOD.Studio.EventInstance _breathInstance;
    private bool _breathPlaying = false;

    public void PlaySkeletonBreath()
    {
        if (_breathPlaying) return;

        _breathInstance = RuntimeManager.CreateInstance(FmodEvents.Instance.SkeletonBreath);
        _breathInstance.set3DAttributes(RuntimeUtils.To3DAttributes(gameObject));
        _breathInstance.start();
        _breathPlaying = true;
    }

    public void StopSkeletonBreath()
    {
        if (!_breathPlaying) return;

        _breathInstance.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT);
        _breathInstance.release();
        _breathPlaying = false;
    }

    public void Update()
    {
        if (_breathPlaying)
        {
            _breathInstance.set3DAttributes(FMODUnity.RuntimeUtils.To3DAttributes(gameObject));
        }
    }
    
    public void StopmonsterAudio()
    {
        StopSkeletonBreath();
        _breathInstance.release();
    }
}

