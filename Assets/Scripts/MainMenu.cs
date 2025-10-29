using System;
using UnityEngine;
using UnityEngine.XR.Interaction.Toolkit.Samples.StarterAssets;

public class MainMenu : MonoBehaviour
{
    public GameObject XROrigin;
    private DynamicMoveProvider moveProvider;
    public float moveSpeed = 1.5f;

    private void Start()
    {
        moveProvider = XROrigin.GetComponent<DynamicMoveProvider>();
        moveProvider.moveSpeed = 0;
    }

    public void StartGame()
    {
        moveProvider.moveSpeed = moveSpeed;
    }
}
