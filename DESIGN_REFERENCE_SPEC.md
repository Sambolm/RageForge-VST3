# RageForge Visual Fidelity Specification

## Authoritative target
The generated RageForge black-and-electric-green interface image supplied in this conversation is the desired design reference. This is a **design specification**, not a claim that the existing C++ rendering is pixel-identical.

## Branding
- Primary title: RAGEFORGE, oversized angular techno/dubstep display typography with sharp bevels and lime glow.
- Producer credit: Sik Rage Productions. No visible mention of Eternatones.
- Black near #050905 base, green #76FF29 principal accent, pale lime #B5FF6A highlight, dark forest green panels.

## Layout
- Strong top masthead and luminous angled green/black frame.
- Upper modules: oscillator visuals, central bright waveform, tempo-synced LFO section with working rate and shape dropdowns.
- Lower modules: grouped synthesizer controls in compact rows. Layout must be reference-proportioned rather than a generic 6-column grid; no clipped controls.
- Rotary controls: black machined dial face, bright lime arc, luminous pointer, crisp light-green caption, numeric value directly underneath.
- Layered beveled contours, fine-lined industrial borders, micro-pattern highlights. Avoid rounded pastel dashboard cards.
- Exact reference font may require a separately licensed display font, or reproduction using original vector lettering. Never distribute unrelated proprietary font files.

## Functionality
- All sound parameters remain real JUCE controls connected through APVTS; no static screenshot pretending to be a functional synthesizer.
- LFO duration subdivisions and shapes remain functional and reflect their true values.
- All visible numeric slider readouts must use juce::String(value,2), not JUCE's default floating-point formatter. Example: 0.50 or 2400.00, never 0.500000.
- Presets, state saving/loading, audio playback, automation, and FL Studio recognition remain unchanged.
- Reference waveform display must be described honestly if it is a parameter preview rather than a real audio oscilloscope.

## Responsive window
- Default must fit laptop displays in FL Studio (currently 890 x 659).
- Resize handle enabled; scale every live control and painted panel with the same letterboxed transform.
- Minimum window 740 x 548, maximum 1450 x 1074; all lower controls must remain visible.
- Resize at multiple FL Studio display scale factors and verify no clipped labels.

## Acceptance criteria
1. Side-by-side screenshot of the compiled VST3 editor and the exact original reference at the same proportions.
2. Compare title typography, stroke shapes, panel positions, knob geometry, LFO panel, waveform graphic, and footer.
3. All readouts show max two decimals at load, during drag, after automation, and after preset changes.
4. Sik Rage Productions is the only visible company branding.
5. Test every knob, preset, LFO dropdown, save/reload, and resize in FL Studio.
6. Keep the plugin build artifact and source revision linked to the same successful GitHub Actions run.

## Current status
The initial custom-drawn JUCE black/green layout is not pixel-perfect. The two-decimal formatter and new branding are implemented. Full image-matched custom artwork and exact font recreation remain future visual fidelity work.
