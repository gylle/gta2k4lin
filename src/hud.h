int hud_init();
void hud_render();

void hud_printf(const char *fmt, ...);
void hud_set_damage(int new_damage);
void hud_set_score(int new_score);
void hud_show_input_field(int onoff);
int hud_input_field_active();
void hud_update_input_field(const char *text);
