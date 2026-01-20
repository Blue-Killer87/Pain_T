#include <gtk/gtk.h>
#include "../core/canvas.h"
#include "../core/brush.h"
#include "../core/tool.h"
#include "../core/theme.h"
#include "../core/tool_factory.h"
#include "../core/tool_id.h"
#include "../core/history.h"

static Canvas* canvas = NULL;
static Tool* current_tool = NULL;
static ToolID current_tool_id = TOOL_BRUSH;
static gboolean drawing = FALSE;
static Theme* current_theme = &THEME_LIGHT;
static GtkWidget *btn_undo = NULL;
static GtkWidget *btn_redo = NULL;
static GtkWidget* color_button = NULL;
static GtkWidget* size_slider;

static void update_color_button(void);
static void update_size_slider(void);

static gboolean on_canvas_resize(GtkWidget* widget, GdkEventConfigure* event, gpointer data);

static gboolean on_canvas_resize(GtkWidget* widget,
                                 GdkEventConfigure* event,
                                 gpointer data)
{
    (void)widget;
    (void)data;

    int new_w = event->width;
    int new_h = event->height;

    if (!canvas || (canvas->width == new_w && canvas->height == new_h))
        return FALSE;

    Canvas* new_canvas = canvas_create(new_w, new_h);

    // fill background with current theme
    canvas_clear(new_canvas, current_theme->background);

    // copy old content
    int copy_w = (new_w < canvas->width) ? new_w : canvas->width;
    int copy_h = (new_h < canvas->height) ? new_h : canvas->height;

    for (int y = 0; y < copy_h; y++) {
        memcpy(
            &new_canvas->pixels[y * new_w],
            &canvas->pixels[y * canvas->width],
            copy_w * sizeof(uint32_t)
        );
    }

    canvas_destroy(canvas);
    canvas = new_canvas;

    return TRUE;
}

static void apply_theme_css(GtkWidget* window, Theme* theme) {
    GtkCssProvider* css = gtk_css_provider_new();

    const char* dark_css =
        "window { background: #1e1e1e; }"
        "box { background: #2a2a2a; }"
        "button { background: #3a3a3a; color: white; }";

    const char* light_css =
        "window { background: #ffffff; }"
        "box { background: #e5e5e5; }"
        "button { background: #f5f5f5; color: black; }";

    gtk_css_provider_load_from_data(
        css,
        (theme == &THEME_DARK) ? dark_css : light_css,
        -1,
        NULL
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(css);
}

static void update_history_buttons(void) {
    gtk_widget_set_sensitive(btn_undo, history_can_undo());
    gtk_widget_set_sensitive(btn_redo, history_can_redo());
}

static void pick_color_at(int x, int y) {
    if (!current_tool || !current_tool->uses_color)
        return;

    uint32_t color = canvas_get_pixel(canvas, x, y);
    current_tool->set_color(current_tool, color);

    /* sync color button UI */
    GdkRGBA rgba = {
        .red   = ((color >> 16) & 0xFF) / 255.0,
        .green = ((color >> 8)  & 0xFF) / 255.0,
        .blue  = ((color)       & 0xFF) / 255.0,
        .alpha = ((color >> 24) & 0xFF) / 255.0
    };

    gtk_color_chooser_set_rgba(
        GTK_COLOR_CHOOSER(color_button), &rgba);
}


static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    (void)widget;
    (void)data;
    cairo_surface_t *surface =
        cairo_image_surface_create_for_data(
            (unsigned char*)canvas->pixels,
            CAIRO_FORMAT_ARGB32,
            canvas->width,
            canvas->height,
            canvas->width * 4
        );

    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);

    cairo_surface_destroy(surface);
    return FALSE;
}
static gboolean on_button_press(GtkWidget *widget,
                                GdkEventButton *event,
                                gpointer data)
{
    (void)data;

    /* Right-click = color picker */
    if (event->button == 3) {
        pick_color_at((int)event->x, (int)event->y);
        return TRUE;
    }

    /* Left-click = draw */
    if (event->button != 1)
        return FALSE;

    history_push(canvas);
    update_history_buttons();

    drawing = TRUE;
    current_tool->on_press(
        current_tool, canvas, event->x, event->y);

    gtk_widget_queue_draw(widget);
    return TRUE;
}

static gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    (void)widget;
    (void)data;
    drawing = FALSE;
    current_tool->on_release(current_tool, canvas, event->x, event->y);
    return TRUE;
}

static gboolean on_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    (void)data;

    if (!drawing) return FALSE;
    current_tool->on_drag(current_tool, canvas, event->x, event->y);
    gtk_widget_queue_draw(widget);
    return TRUE;
}

static gboolean on_key_press(GtkWidget* w, GdkEventKey* e, gpointer data) {
    (void)data;

    if ((e->state & GDK_CONTROL_MASK) && e->keyval == GDK_KEY_z) {
        if (history_undo(canvas))
            gtk_widget_queue_draw(w);
        return TRUE;
    }

    if ((e->state & GDK_CONTROL_MASK) && e->keyval == GDK_KEY_y) {
        if (history_redo(canvas))
            gtk_widget_queue_draw(w);
        return TRUE;
    }

    if (e->keyval == GDK_KEY_t) {
        current_theme = (current_theme == &THEME_LIGHT)
                        ? &THEME_DARK
                        : &THEME_LIGHT;

        canvas_clear(canvas, current_theme->background);
        current_tool->set_color(current_tool, current_theme->foreground);



        gtk_widget_queue_draw(w);
    }
    return FALSE;
}

static void update_color_button(void) {
    if (!color_button)
        return;

    if (current_tool && current_tool->uses_color) {
        gtk_widget_set_sensitive(color_button, TRUE);

        uint32_t color = current_tool->get_color(current_tool);


        GdkRGBA rgba = {
            .red   = ((color >> 16) & 0xFF) / 255.0,
            .green = ((color >> 8)  & 0xFF) / 255.0,
            .blue  = ((color)       & 0xFF) / 255.0,
            .alpha = ((color >> 24) & 0xFF) / 255.0
        };


        gtk_color_chooser_set_rgba(
            GTK_COLOR_CHOOSER(color_button), &rgba);
    } else {
        gtk_widget_set_sensitive(color_button, FALSE);
    }
}

static void switch_tool(ToolID id) {
    if (current_tool)
        tool_destroy(current_tool);

    current_tool_id = id;
    current_tool = tool_create(id, current_theme);

    update_color_button();
    update_size_slider();
}

static void on_tool_brush(GtkWidget* w, gpointer d) {
    (void)w; (void)d;
    switch_tool(TOOL_BRUSH);
}

static void on_tool_eraser(GtkWidget* w, gpointer d) {
    (void)w; (void)d;
    switch_tool(TOOL_ERASER);
}

static void on_tool_fill(GtkButton* b, gpointer data) {
    (void)b; (void)data;
    switch_tool(TOOL_FILL);
}

static void on_undo(GtkButton* b, gpointer data) {
    (void)b;
    GtkWidget* area = GTK_WIDGET(data);

    if (history_undo(canvas))
        gtk_widget_queue_draw(area);

    update_history_buttons();
}

static void on_redo(GtkButton* b, gpointer data) {
    (void)b;
    GtkWidget* area = GTK_WIDGET(data);

    if (history_redo(canvas))
        gtk_widget_queue_draw(area);

    update_history_buttons();
}

static void on_toggle_theme(GtkButton* b, gpointer data) {
    (void)b;

    GtkWidget* window = GTK_WIDGET(data);

    // Toggle theme
    current_theme = (current_theme == &THEME_LIGHT)
                    ? &THEME_DARK
                    : &THEME_LIGHT;

    // Apply theme CSS to window and toolbar
    apply_theme_css(window, current_theme);
}

static void on_color_changed(GtkColorButton* btn, gpointer user_data) {
    (void)user_data;
    if (!current_tool || !current_tool->uses_color)
        return;

    GdkRGBA rgba;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(btn), &rgba);

    uint32_t color =
        ((uint8_t)(rgba.alpha * 255) << 24) |
        ((uint8_t)(rgba.red   * 255) << 16) |
        ((uint8_t)(rgba.green * 255) << 8)  |
        ((uint8_t)(rgba.blue  * 255));

    current_tool->set_color(current_tool, color);

}

static void on_save(GtkButton* b, gpointer data) {
    (void)b;

    GtkWindow* parent = GTK_WINDOW(data);

    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Save Image",
        parent,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_ACCEPT,
        NULL
    );

    gtk_file_chooser_set_do_overwrite_confirmation(
        GTK_FILE_CHOOSER(dialog), TRUE);

    gtk_file_chooser_set_current_name(
        GTK_FILE_CHOOSER(dialog), "image.png");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename =
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        canvas_save_png(canvas, filename);

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

static void on_size_changed(GtkRange* range, gpointer user_data) {
    (void)user_data;
    if (!current_tool || !current_tool->set_size)
        return;

    int size = (int)gtk_range_get_value(range);
    current_tool->set_size(current_tool, size);
}

static void update_size_slider(void) {
    if (!size_slider || !current_tool)
        return;

    if (!current_tool->supports_size) {
        gtk_widget_set_sensitive(size_slider, FALSE);
        return;
    }

    gtk_widget_set_sensitive(size_slider, TRUE);

    int size = tool_get_size(current_tool);
    gtk_range_set_value(GTK_RANGE(size_slider), size);
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);
    history_init();

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Paint C");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *area = gtk_drawing_area_new();

    gtk_widget_set_hexpand(toolbar, FALSE);
    gtk_widget_set_size_request(toolbar, 60, -1);

    // Canvas fills all available space
    gtk_widget_set_hexpand(area, TRUE);
    gtk_widget_set_vexpand(area, TRUE);

    // Pack toolbar and canvas
    gtk_box_pack_start(GTK_BOX(root), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), area, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(window), root);
    
    // Create canvas before tools
    canvas = canvas_create(800, 600);
    canvas_clear(canvas, current_theme->background);

    // Apply initial theme
    apply_theme_css(window, current_theme);

    // Set up initial tool
    switch_tool(TOOL_BRUSH);

    // Add canvas events
    gtk_widget_add_events(area,
        GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK |
        GDK_POINTER_MOTION_MASK
    );

    // Brush size slider
    size_slider = gtk_scale_new_with_range(
        GTK_ORIENTATION_HORIZONTAL, 1, 100, 1);
    gtk_scale_set_draw_value(GTK_SCALE(size_slider), TRUE);
    gtk_widget_set_sensitive(size_slider, FALSE);
    gtk_widget_set_hexpand(size_slider, TRUE);

    // Toolbar buttons
    GtkWidget* btn_brush  = gtk_button_new_with_label("Brush");
    GtkWidget* btn_eraser = gtk_button_new_with_label("Eraser");
    GtkWidget* btn_fill   = gtk_button_new_with_label("Fill");
    btn_undo = gtk_button_new_with_label("Undo");
    btn_redo = gtk_button_new_with_label("Redo");
    GtkWidget* btn_save   = gtk_button_new_with_label("Save");
    GtkWidget* btn_theme  = gtk_button_new_with_label("Theme");

    gtk_box_pack_start(GTK_BOX(toolbar), btn_brush,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_eraser, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_fill,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_undo,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_redo,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_save,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_theme,  FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(toolbar), size_slider, FALSE, FALSE, 4);

    // Spacer pushes color picker down
    GtkWidget* spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(spacer, TRUE);
    gtk_box_pack_start(GTK_BOX(toolbar), spacer, TRUE, TRUE, 0);

    // Color picker
    color_button = gtk_color_button_new();
    gtk_widget_set_sensitive(color_button, FALSE);
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(color_button), TRUE);
    gtk_box_pack_start(GTK_BOX(toolbar), color_button, FALSE, FALSE, 6);

    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);

    // Connect signals
    g_signal_connect(area, "configure-event", G_CALLBACK(on_canvas_resize), NULL);
    g_signal_connect(area, "draw", G_CALLBACK(on_draw), NULL);
    g_signal_connect(area, "button-press-event", G_CALLBACK(on_button_press), NULL);
    g_signal_connect(area, "button-release-event", G_CALLBACK(on_button_release), NULL);
    g_signal_connect(area, "motion-notify-event", G_CALLBACK(on_motion), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    g_signal_connect(btn_brush,  "clicked", G_CALLBACK(on_tool_brush), NULL);
    g_signal_connect(btn_eraser, "clicked", G_CALLBACK(on_tool_eraser), NULL);
    g_signal_connect(btn_fill,   "clicked", G_CALLBACK(on_tool_fill), NULL);
    g_signal_connect(btn_undo,   "clicked", G_CALLBACK(on_undo), area);
    g_signal_connect(btn_redo,   "clicked", G_CALLBACK(on_redo), area);
    g_signal_connect(btn_save,   "clicked", G_CALLBACK(on_save), window);
    g_signal_connect(btn_theme,  "clicked", G_CALLBACK(on_toggle_theme), window);

    g_signal_connect(color_button, "color-set", G_CALLBACK(on_color_changed), NULL);
    g_signal_connect(size_slider, "value-changed", G_CALLBACK(on_size_changed), NULL);

    // Update UI states
    update_color_button();
    update_size_slider();

    gtk_widget_show_all(window);
    gtk_main();

    history_free();
    tool_destroy(current_tool);
    canvas_destroy(canvas);

    return 0;
}
