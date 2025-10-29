using UnityEngine;
using DG.Tweening;
using UnityEngine.XR.Interaction.Toolkit.Locomotion.Comfort;
using System.Data;


public class BloodVignette : MonoBehaviour, ITunnelingVignetteProvider
{

    public PlayerHealthSystem playerHealthSystem;

    public Material bloodVignetteMaterial;

    [Header("Tweening Settings")]
    public float materialCutoffEaseValue = 0.5f;
    public float apertureEaseValue = 0.5f;

    [Header("Vignette Ranges")]
    [Tooltip("Minimum cutoff value (full health, no vignette)")]
    public float minCutoff = 1f;
    [Tooltip("Maximum cutoff value (low health, strong vignette)")]
    public float maxCutoff = 0.3f;
    [Tooltip("Minimum aperture size (full health, no tunnel)")]
    public float minAperture = 1f;
    [Tooltip("Maximum aperture size (low health, tight tunnel)")]
    public float maxAperture = 0.75f;

    private const float FullHealth = 1f; // The full health percentage value of the player should be 1.0 (100%)

    private TunnelingVignetteController tunnelVignetteController;

    // Vignette parameters we provide to the TunnelingVignetteController
    VignetteParameters m_VignetteParameters = new VignetteParameters();
    bool m_RegisteredWithController;

    // Current values for smooth tweening
    float currentCutoff = 1f;
    float currentAperture = 1f;

    void Start()
    {
        InitializeVignetteParams();

        tunnelVignetteController = GetComponent<TunnelingVignetteController>();
        // Fallback: try to find one in the scene if not attached to same GameObject
        if (tunnelVignetteController == null)
            tunnelVignetteController = FindObjectOfType<TunnelingVignetteController>();
    }

    private void InitializeVignetteParams()
    {
        // Initialize vignette parameters
        m_VignetteParameters.apertureSize = currentAperture;
        bloodVignetteMaterial.SetFloat("_Cutoff", currentCutoff);
    }

    public void UpdateVignetteParameters()
    {
        float healthPercent = playerHealthSystem.GetHealthPercentage();

        // Calculate target values based on health (lower health = stronger vignette)
        float targetCutoff = Mathf.Lerp(minCutoff, maxCutoff, FullHealth - healthPercent); // goes from min to max as health goes from full to zero (100% health to 0%)
        float targetAperture = Mathf.Lerp(minAperture, maxAperture, FullHealth - healthPercent);

        // Smoothly tween the cutoff on the material
        DOVirtual.Float(currentCutoff, targetCutoff, materialCutoffEaseValue, value =>
        {
            currentCutoff = value;
            bloodVignetteMaterial.SetFloat("_Cutoff", value);
        }).SetEase(Ease.OutQuad);

        // Smoothly tween the aperture size for the tunneling vignette
        DOVirtual.Float(currentAperture, targetAperture, apertureEaseValue, value =>
        {
            currentAperture = value;
            m_VignetteParameters.apertureSize = value;
        }).SetEase(Ease.OutQuad);
    }

    void OnEnable()
    {
        // Register this provider with the controller so it will be used to control the vignette
        if (!m_RegisteredWithController)
        {
            if (tunnelVignetteController == null)
                tunnelVignetteController = GetComponent<TunnelingVignetteController>() ?? FindObjectOfType<TunnelingVignetteController>();

            if (tunnelVignetteController != null)
            {
                tunnelVignetteController.BeginTunnelingVignette(this);
                m_RegisteredWithController = true;
            }
        }
    }

    void OnDisable()
    {
        if (m_RegisteredWithController && tunnelVignetteController != null)
        {
            tunnelVignetteController.EndTunnelingVignette(this);
            m_RegisteredWithController = false;
        }
    }

    // ITunnelingVignetteProvider implementation - controller will read this
    public VignetteParameters vignetteParameters => m_VignetteParameters;
}
