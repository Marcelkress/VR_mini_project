using Unity.XR.CoreUtils;
using UnityEngine;

public class XROriginReset : MonoBehaviour
{
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        GetComponent<XROrigin>().MoveCameraToWorldLocation(this.transform.position);
    }

    
}
