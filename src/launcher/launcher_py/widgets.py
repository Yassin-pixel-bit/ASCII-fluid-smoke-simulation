def slider_decimals(max_val: float) -> int:
    """Return the appropriate decimal precision for a slider based on its max value."""
    return 2 if max_val <= 1.0 else 1


def link_slider_and_entry(slider_var, entry_var, min_val, max_val, decimals):
    """
    Bind a CTkSlider and a CTkEntry so they stay in sync.
    Returns (on_slider_move, validate_entry) to be wired up by the caller.
    """
    is_updating = False

    def on_slider_move(new_val):
        nonlocal is_updating
        if not is_updating:
            is_updating = True
            entry_var.set(f"{float(new_val):.{decimals}f}")
            is_updating = False

    def on_text_change(*args):
        nonlocal is_updating
        if not is_updating:
            user_text = entry_var.get()
            if user_text in ("", "-", ".", "-."):
                return
            try:
                user_val = float(user_text)
                clamped_val = max(min_val, min(user_val, max_val))
                is_updating = True
                slider_var.set(clamped_val)
                is_updating = False
            except ValueError:
                pass

    def validate_entry(event=None):
        nonlocal is_updating
        is_updating = True
        try:
            user_val = float(entry_var.get())
            clamped_val = max(min_val, min(user_val, max_val))
            slider_var.set(clamped_val)
            entry_var.set(f"{clamped_val:.{decimals}f}")
        except ValueError:
            entry_var.set(f"{slider_var.get():.{decimals}f}")
        is_updating = False

    entry_var.trace_add("write", on_text_change)
    return on_slider_move, validate_entry