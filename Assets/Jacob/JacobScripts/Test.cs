using UnityEngine;
using System.Collections;

public class Test : MonoBehaviour
{
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {

        OVRPlugin.foveatedRenderingLevel = OVRPlugin.FoveatedRenderingLevel.High;
    }
}
