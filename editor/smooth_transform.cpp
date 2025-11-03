/**
 * Implementação do SmoothTransform
 */

#include "smooth_transform.h"
#include <wx/log.h>
#include <cmath>
#include <algorithm>

// ============================================================================
// SmoothTransform
// ============================================================================

SmoothTransform::SmoothTransform()
    : m_currentZoom(1.0f)
    , m_currentPanX(0.0f)
    , m_currentPanY(0.0f)
    , m_targetZoom(1.0f)
    , m_targetPanX(0.0f)
    , m_targetPanY(0.0f)
    , m_startZoom(1.0f)
    , m_startPanX(0.0f)
    , m_startPanY(0.0f)
    , m_isAnimating(false)
    , m_animateZoom(false)
    , m_animatePan(false)
    , m_animDuration(300)
    , m_easingType(EasingType::EaseInOut)
    , m_zoomIncrement(0.1f)
{
}

SmoothTransform::~SmoothTransform()
{
    StopAnimation();
}

void SmoothTransform::SetTransform(float zoom, float panX, float panY)
{
    StopAnimation();
    
    m_currentZoom = zoom;
    m_currentPanX = panX;
    m_currentPanY = panY;
    
    ApplyBounds(m_currentZoom, m_currentPanX, m_currentPanY);
    
    // Notificar callback
    if (m_callback) {
        m_callback(m_currentZoom, m_currentPanX, m_currentPanY);
    }
}

void SmoothTransform::SetBounds(const TransformBounds& bounds)
{
    m_bounds = bounds;
    
    // Aplicar limites aos valores atuais
    ApplyBounds(m_currentZoom, m_currentPanX, m_currentPanY);
}

void SmoothTransform::AnimateZoom(float targetZoom, int durationMs, EasingType easing)
{
    // Aplicar limites ao target
    ApplyBounds(targetZoom, m_currentPanX, m_currentPanY);
    
    if (std::abs(m_currentZoom - targetZoom) < 0.001f) {
        return; // Já está no target
    }
    
    StopAnimation();
    
    m_startZoom = m_currentZoom;
    m_targetZoom = targetZoom;
    m_animateZoom = true;
    m_animatePan = false;
    m_animDuration = durationMs;
    m_easingType = easing;
    m_isAnimating = true;
    m_animStartTime = std::chrono::high_resolution_clock::now();
    
    // Iniciar timer (16ms ≈ 60 FPS)
    Start(16, wxTIMER_CONTINUOUS);
}

void SmoothTransform::AnimatePan(float targetX, float targetY, int durationMs, EasingType easing)
{
    // Aplicar limites ao target
    float tempZoom = m_currentZoom;
    ApplyBounds(tempZoom, targetX, targetY);
    
    if (std::abs(m_currentPanX - targetX) < 0.1f && std::abs(m_currentPanY - targetY) < 0.1f) {
        return; // Já está no target
    }
    
    StopAnimation();
    
    m_startPanX = m_currentPanX;
    m_startPanY = m_currentPanY;
    m_targetPanX = targetX;
    m_targetPanY = targetY;
    m_animateZoom = false;
    m_animatePan = true;
    m_animDuration = durationMs;
    m_easingType = easing;
    m_isAnimating = true;
    m_animStartTime = std::chrono::high_resolution_clock::now();
    
    Start(16, wxTIMER_CONTINUOUS);
}

void SmoothTransform::AnimateZoomAndPan(float targetZoom, float targetX, float targetY, 
                                         int durationMs, EasingType easing)
{
    // Aplicar limites ao target
    ApplyBounds(targetZoom, targetX, targetY);
    
    StopAnimation();
    
    m_startZoom = m_currentZoom;
    m_startPanX = m_currentPanX;
    m_startPanY = m_currentPanY;
    m_targetZoom = targetZoom;
    m_targetPanX = targetX;
    m_targetPanY = targetY;
    m_animateZoom = true;
    m_animatePan = true;
    m_animDuration = durationMs;
    m_easingType = easing;
    m_isAnimating = true;
    m_animStartTime = std::chrono::high_resolution_clock::now();
    
    Start(16, wxTIMER_CONTINUOUS);
}

void SmoothTransform::StopAnimation()
{
    if (m_isAnimating) {
        Stop();
        m_isAnimating = false;
        m_animateZoom = false;
        m_animatePan = false;
    }
}

void SmoothTransform::ZoomIn()
{
    float targetZoom = m_currentZoom + m_zoomIncrement;
    AnimateZoom(targetZoom, 200);
}

void SmoothTransform::ZoomOut()
{
    float targetZoom = m_currentZoom - m_zoomIncrement;
    AnimateZoom(targetZoom, 200);
}

void SmoothTransform::ZoomToPoint(float targetZoom, float worldX, float worldY, 
                                   int viewportWidth, int viewportHeight)
{
    // Calcular novo pan para manter o ponto do mundo sob o cursor
    // Formula: novo_pan = ponto_mundo - (ponto_tela / novo_zoom)
    
    (void)viewportWidth;  // Não usado nesta implementação
    (void)viewportHeight; // Não usado nesta implementação
    
    // Posição atual do ponto na tela (relativo ao pan atual)
    float screenX = (worldX - m_currentPanX) * m_currentZoom;
    float screenY = (worldY - m_currentPanY) * m_currentZoom;
    
    // Nova posição do ponto para manter ele no mesmo lugar da tela
    float newPanX = worldX - (screenX / targetZoom);
    float newPanY = worldY - (screenY / targetZoom);
    
    AnimateZoomAndPan(targetZoom, newPanX, newPanY, 300);
}

void SmoothTransform::FitToView(float contentWidth, float contentHeight, 
                                 int viewportWidth, int viewportHeight)
{
    if (contentWidth <= 0 || contentHeight <= 0 || viewportWidth <= 0 || viewportHeight <= 0) {
        return;
    }
    
    // Calcular zoom para caber o conteúdo no viewport
    float zoomX = viewportWidth / contentWidth;
    float zoomY = viewportHeight / contentHeight;
    float targetZoom = std::min(zoomX, zoomY) * 0.9f; // 90% para dar margem
    
    // Centralizar
    float targetPanX = (contentWidth - viewportWidth / targetZoom) / 2.0f;
    float targetPanY = (contentHeight - viewportHeight / targetZoom) / 2.0f;
    
    AnimateZoomAndPan(targetZoom, targetPanX, targetPanY, 400);
}

void SmoothTransform::ResetZoom()
{
    AnimateZoom(1.0f, 300);
}

void SmoothTransform::Notify()
{
    UpdateAnimation();
}

void SmoothTransform::UpdateAnimation()
{
    if (!m_isAnimating) {
        return;
    }
    
    // Calcular progresso (0.0 - 1.0)
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_animStartTime);
    float progress = static_cast<float>(elapsed.count()) / m_animDuration;
    
    if (progress >= 1.0f) {
        // Animação completa
        if (m_animateZoom) {
            m_currentZoom = m_targetZoom;
        }
        if (m_animatePan) {
            m_currentPanX = m_targetPanX;
            m_currentPanY = m_targetPanY;
        }
        
        StopAnimation();
        
        // Notificar callback
        if (m_callback) {
            m_callback(m_currentZoom, m_currentPanX, m_currentPanY);
        }
        
        return;
    }
    
    // Aplicar easing
    float t = ApplyEasing(progress);
    
    // Interpolar valores
    if (m_animateZoom) {
        m_currentZoom = Interpolate(t, m_startZoom, m_targetZoom);
    }
    if (m_animatePan) {
        m_currentPanX = Interpolate(t, m_startPanX, m_targetPanX);
        m_currentPanY = Interpolate(t, m_startPanY, m_targetPanY);
    }
    
    // Aplicar limites
    ApplyBounds(m_currentZoom, m_currentPanX, m_currentPanY);
    
    // Notificar callback
    if (m_callback) {
        m_callback(m_currentZoom, m_currentPanX, m_currentPanY);
    }
}

void SmoothTransform::ApplyBounds(float& zoom, float& panX, float& panY)
{
    if (!m_bounds.enableBounds) {
        return;
    }
    
    // Limitar zoom
    zoom = std::max(m_bounds.minZoom, std::min(m_bounds.maxZoom, zoom));
    
    // Limitar pan
    panX = std::max(m_bounds.minPanX, std::min(m_bounds.maxPanX, panX));
    panY = std::max(m_bounds.minPanY, std::min(m_bounds.maxPanY, panY));
}

float SmoothTransform::Interpolate(float t, float start, float end)
{
    return start + (end - start) * t;
}

float SmoothTransform::ApplyEasing(float t)
{
    // Clampar t entre 0 e 1
    t = std::max(0.0f, std::min(1.0f, t));
    
    switch (m_easingType) {
        case EasingType::Linear:
            return t;
            
        case EasingType::EaseIn:
            // Quadratic ease-in: t^2
            return t * t;
            
        case EasingType::EaseOut:
            // Quadratic ease-out: 1 - (1-t)^2
            return 1.0f - (1.0f - t) * (1.0f - t);
            
        case EasingType::EaseInOut:
        default:
            // Quadratic ease-in-out
            if (t < 0.5f) {
                return 2.0f * t * t;
            } else {
                float f = t - 1.0f;
                return 1.0f - 2.0f * f * f;
            }
    }
}
