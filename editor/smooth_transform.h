/**
 * SmoothTransform - Sistema de transformação suave para zoom e pan
 * Usa interpolação ease-in-out para transições animadas
 */

#pragma once

#include <wx/timer.h>
#include <chrono>

/**
 * Tipo de easing para animações
 */
enum class EasingType {
    Linear,        // Interpolação linear
    EaseInOut,     // Suave no início e fim (padrão)
    EaseIn,        // Suave no início
    EaseOut        // Suave no fim
};

/**
 * Limites de transformação
 */
struct TransformBounds {
    float minZoom;     // Zoom mínimo (padrão: 0.25x)
    float maxZoom;     // Zoom máximo (padrão: 4.0x)
    float minPanX;     // Pan X mínimo
    float maxPanX;     // Pan X máximo
    float minPanY;     // Pan Y mínimo
    float maxPanY;     // Pan Y máximo
    bool enableBounds; // Se os limites estão ativos
    
    TransformBounds()
        : minZoom(0.25f)
        , maxZoom(4.0f)
        , minPanX(0.0f)
        , maxPanX(1000.0f)
        , minPanY(0.0f)
        , maxPanY(1000.0f)
        , enableBounds(true)
    {}
};

/**
 * SmoothTransform - Gerencia transformações animadas de zoom e pan
 * 
 * Funcionalidades:
 * - Interpolação suave (ease-in-out) para zoom e pan
 * - Animações temporais precisas
 * - Limites configuráveis
 * - Callbacks para atualização
 * - Suporte a cancelamento de animações
 */
class SmoothTransform : public wxTimer {
public:
    /**
     * Callback para notificar mudanças de transformação
     */
    using UpdateCallback = std::function<void(float zoom, float panX, float panY)>;
    
    /**
     * Construtor
     */
    SmoothTransform();
    
    /**
     * Destrutor
     */
    ~SmoothTransform();
    
    /**
     * Inicia animação de zoom suave
     * 
     * @param targetZoom Zoom de destino
     * @param durationMs Duração da animação em milissegundos
     * @param easing Tipo de easing
     */
    void AnimateZoom(float targetZoom, int durationMs = 300, EasingType easing = EasingType::EaseInOut);
    
    /**
     * Inicia animação de pan suave
     * 
     * @param targetX Pan X de destino
     * @param targetY Pan Y de destino
     * @param durationMs Duração da animação em milissegundos
     * @param easing Tipo de easing
     */
    void AnimatePan(float targetX, float targetY, int durationMs = 200, EasingType easing = EasingType::EaseInOut);
    
    /**
     * Inicia animação de zoom e pan simultaneamente
     * 
     * @param targetZoom Zoom de destino
     * @param targetX Pan X de destino
     * @param targetY Pan Y de destino
     * @param durationMs Duração da animação
     * @param easing Tipo de easing
     */
    void AnimateZoomAndPan(float targetZoom, float targetX, float targetY, 
                           int durationMs = 300, EasingType easing = EasingType::EaseInOut);
    
    /**
     * Define valores atuais de transformação (sem animação)
     */
    void SetTransform(float zoom, float panX, float panY);
    
    /**
     * Retorna valores atuais
     */
    float GetZoom() const { return m_currentZoom; }
    float GetPanX() const { return m_currentPanX; }
    float GetPanY() const { return m_currentPanY; }
    
    /**
     * Define limites de transformação
     */
    void SetBounds(const TransformBounds& bounds);
    
    /**
     * Retorna limites atuais
     */
    const TransformBounds& GetBounds() const { return m_bounds; }
    
    /**
     * Define callback de atualização
     */
    void SetUpdateCallback(UpdateCallback callback) { m_callback = callback; }
    
    /**
     * Verifica se há animação em andamento
     */
    bool IsAnimating() const { return m_isAnimating; }
    
    /**
     * Cancela animação atual
     */
    void StopAnimation();
    
    /**
     * Define incremento de zoom (para mouse wheel)
     * 
     * @param increment Incremento por "tick" do wheel (padrão: 0.1 = 10%)
     */
    void SetZoomIncrement(float increment) { m_zoomIncrement = increment; }
    
    /**
     * Zoom in por um incremento
     */
    void ZoomIn();
    
    /**
     * Zoom out por um incremento
     */
    void ZoomOut();
    
    /**
     * Zoom para um ponto específico (zoom centrado)
     * 
     * @param targetZoom Zoom de destino
     * @param worldX Posição X no mundo (antes do zoom)
     * @param worldY Posição Y no mundo (antes do zoom)
     * @param viewportWidth Largura do viewport
     * @param viewportHeight Altura do viewport
     */
    void ZoomToPoint(float targetZoom, float worldX, float worldY, 
                     int viewportWidth, int viewportHeight);
    
    /**
     * Ajusta para caber todo o conteúdo no viewport
     * 
     * @param contentWidth Largura do conteúdo
     * @param contentHeight Altura do conteúdo
     * @param viewportWidth Largura do viewport
     * @param viewportHeight Altura do viewport
     */
    void FitToView(float contentWidth, float contentHeight, 
                   int viewportWidth, int viewportHeight);
    
    /**
     * Reset para zoom 100%
     */
    void ResetZoom();

private:
    // wxTimer override
    void Notify() override;
    
    // Estado atual
    float m_currentZoom;
    float m_currentPanX;
    float m_currentPanY;
    
    // Estado alvo
    float m_targetZoom;
    float m_targetPanX;
    float m_targetPanY;
    
    // Estado inicial (para interpolação)
    float m_startZoom;
    float m_startPanX;
    float m_startPanY;
    
    // Controle de animação
    bool m_isAnimating;
    bool m_animateZoom;
    bool m_animatePan;
    std::chrono::high_resolution_clock::time_point m_animStartTime;
    int m_animDuration; // Em milissegundos
    EasingType m_easingType;
    
    // Limites
    TransformBounds m_bounds;
    
    // Callback
    UpdateCallback m_callback;
    
    // Configuração
    float m_zoomIncrement; // Incremento de zoom (padrão: 0.1)
    
    /**
     * Atualiza animação
     */
    void UpdateAnimation();
    
    /**
     * Aplica limites aos valores
     */
    void ApplyBounds(float& zoom, float& panX, float& panY);
    
    /**
     * Calcula valor interpolado com easing
     * 
     * @param t Progresso normalizado (0.0 - 1.0)
     * @param start Valor inicial
     * @param end Valor final
     * @return Valor interpolado
     */
    float Interpolate(float t, float start, float end);
    
    /**
     * Aplica função de easing ao progresso
     * 
     * @param t Progresso linear (0.0 - 1.0)
     * @return Progresso com easing aplicado
     */
    float ApplyEasing(float t);
};
