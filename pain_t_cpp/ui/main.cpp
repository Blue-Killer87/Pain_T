#include <gtk/gtk.h>
#include <memory>
#include <algorithm>

#include "../core/canvas.h"
#include "../core/brush.h"
#include "../core/fill.h"
#include "../core/history.h"
#include "../core/theme.h"
#include "../core/tool.h"
#include "../core/image_tool.h"


//----------globals----------
static History history;
static std::unique_ptr<Canvas> canvas;
static std::unique_ptr<Tool> current_tool;
static bool drawing = false;
static Theme* current_theme = &THEME_LIGHT;

static GtkWidget *btn_undo = nullptr;
static GtkWidget *btn_redo = nullptr;
static GtkWidget* color_button = nullptr;
static GtkWidget* size_slider = nullptr;
static GtkWidget* area = nullptr;

//for tool highlighting (maybe redundant?)
static GtkWidget* current_tool_button = nullptr;

//----------------helpers----------------
static void update_color_button();
static void update_size_slider();
static void switch_tool(std::unique_ptr<Tool> tool);
static void on_save(GtkButton* b, gpointer data);
static void highlight_tool(GtkWidget* btn);

static 
void commit_current_tool(){
    if (!current_tool) return;
    current_tool->apply(*canvas);
    switch_tool(std::make_unique<Brush>(0xFF000000, 4));
    gtk_widget_queue_draw(area);
}


//----------GTK callback stuff-----------
static 
gboolean on_canvas_resize(GtkWidget*, GdkEventConfigure* event, gpointer){
    int new_w = event->width;
    int new_h = event->height;
    if (!canvas || (canvas->getWidth() == new_w && canvas->getHeight() == new_h))
        return FALSE;

    auto new_canvas = std::make_unique<Canvas>(new_w, new_h);
    new_canvas->clear(current_theme->background);

    int copy_w = std::min(new_w, canvas->getWidth());
    int copy_h = std::min(new_h, canvas->getHeight());
    for (int y = 0; y < copy_h; y++)
        std::copy_n(canvas->getPixels().data() + y * canvas->getWidth(),
                    copy_w,
                    new_canvas->getPixels().data() + y * new_w);

    canvas = std::move(new_canvas);
    return TRUE;
}

static 
void apply_theme_css(GtkWidget*, Theme* theme){
    GtkCssProvider* css = gtk_css_provider_new();

    const char* dark_css =
        "box{background: #2a2a2a;}"
        "button{background: #3a3a3a; color: white;}"
        "button:disabled{background: #1e1e1e; color: #777;}"
        "button.active{border: 2px solid #ff9900;}";

    const char* light_css =
        "box{background: #e5e5e5;}"
        "button{background: #f5f5f5; color: black;}"
        "button:disabled{background: #cfcfcf; color: #888;}"
        "button.active{border: 2px solid #ff9900;}";

    gtk_css_provider_load_from_data(
        css,
        (theme == &THEME_DARK) ? dark_css : light_css,
        -1,
        nullptr
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(css);
}

static 
void update_history_buttons(){
    gtk_widget_set_sensitive(btn_undo, history.canUndo());
    gtk_widget_set_sensitive(btn_redo, history.canRedo());
}

static 
void pick_color_at(int x, int y){
    if (!current_tool || !current_tool->usesColor()) return;
    uint32_t color = canvas->getPixel(x, y);
    current_tool->setColor(color);

    GdkRGBA rgba ={
        .red   = ((color >> 16) & 0xFF) / 255.0,
        .green = ((color >> 8)  & 0xFF) / 255.0,
        .blue  = ((color)       & 0xFF) / 255.0,
        .alpha = ((color >> 24) & 0xFF) / 255.0
    };
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(color_button), &rgba);
}

static 
void on_save(GtkButton* /*b*/, gpointer data){
    GtkWindow* parent = GTK_WINDOW(data);
    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Save Image", parent, GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_ACCEPT, NULL
    );
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "image.png");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT){
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        canvas->savePNG(filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

static 
gboolean on_draw(GtkWidget*, cairo_t* cr, gpointer){
    if (!canvas) return FALSE;

    cairo_surface_t *surface = cairo_image_surface_create_for_data(
        reinterpret_cast<unsigned char*>(canvas->getPixels().data()),
        CAIRO_FORMAT_ARGB32,
        canvas->getWidth(),
        canvas->getHeight(),
        canvas->getWidth() * 4
    );

    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);

    if (current_tool){
        current_tool->drawOverlay(cr);
    }
    cairo_surface_destroy(surface);
    return FALSE;
}

static 
gboolean on_button_press(GtkWidget*, GdkEventButton* event, gpointer){
    if (event->button == 3){
        pick_color_at((int)event->x, (int)event->y);
        return TRUE;
    }
    if (event->button != 1) return FALSE;

    drawing = true;
    if (!canvas) return FALSE;

    history.push(*canvas);
    update_history_buttons();

    if (current_tool) current_tool->press(*canvas, event->x, event->y);

    gtk_widget_queue_draw(area);
    return TRUE;
}

static 
gboolean on_button_release(GtkWidget*, GdkEventButton* event, gpointer){
    drawing = false;
    if (current_tool) current_tool->release(*canvas, event->x, event->y);
    gtk_widget_queue_draw(area);
    return TRUE;
}

static 
gboolean on_motion(GtkWidget*, GdkEventMotion* event, gpointer){
    if (!drawing || !current_tool) return FALSE;
    current_tool->drag(*canvas, event->x, event->y);
    gtk_widget_queue_draw(area);
    return TRUE;
}

static 
gboolean on_key_press(GtkWidget* w, GdkEventKey* e, gpointer){
    if ((e->state & GDK_CONTROL_MASK) && e->keyval == GDK_KEY_z){
        if (history.undo(*canvas)) gtk_widget_queue_draw(w);
        update_history_buttons();
        return TRUE;
    }
    if ((e->state & GDK_CONTROL_MASK) && e->keyval == GDK_KEY_y){
        if (history.redo(*canvas)) gtk_widget_queue_draw(w);
        update_history_buttons();
        return TRUE;
    }
    if (e->keyval == GDK_KEY_t){
        current_theme = (current_theme == &THEME_LIGHT) ? &THEME_DARK : &THEME_LIGHT;
        apply_theme_css(w, current_theme);
        return TRUE;
    }
    if ((e->state & GDK_CONTROL_MASK) && e->keyval == GDK_KEY_v){
        GtkClipboard* cb = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        if (gtk_clipboard_wait_is_image_available(cb)){
            GdkPixbuf* pixbuf = gtk_clipboard_wait_for_image(cb);
            if (pixbuf){
                switch_tool(std::make_unique<ImageTool>(pixbuf));
            }
        }
        return TRUE;
    }
    if (e->keyval == GDK_KEY_Return){
        commit_current_tool();
        return TRUE;
    }

    return FALSE;
}

//--------------tool/color/size-----------
static void 
update_color_button(){
    if (!color_button) return;

    if (current_tool && current_tool->usesColor()){
        gtk_widget_set_sensitive(color_button, TRUE);
        uint32_t color = current_tool->getColor();
        GdkRGBA rgba ={
            .red   = ((color >> 16) & 0xFF) / 255.0,
            .green = ((color >> 8)  & 0xFF) / 255.0,
            .blue  = ((color)       & 0xFF) / 255.0,
            .alpha = ((color >> 24) & 0xFF) / 255.0
        };
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(color_button), &rgba);
    } else{
        gtk_widget_set_sensitive(color_button, FALSE);
    }
}

static void 
update_size_slider(){
    if (!size_slider || !current_tool) return;
    if (!current_tool->supportsSize()){
        gtk_widget_set_sensitive(size_slider, FALSE);
        return;
    }
    gtk_widget_set_sensitive(size_slider, TRUE);
    gtk_range_set_value(GTK_RANGE(size_slider), current_tool->getSize());
}

static void 
switch_tool(std::unique_ptr<Tool> tool){
    if (current_tool) current_tool->apply(*canvas); 
    current_tool = std::move(tool);
    update_color_button();
    update_size_slider();
    gtk_widget_queue_draw(area);
}


static void 
highlight_tool(GtkWidget* btn){
    if (current_tool_button){
        gtk_style_context_remove_class(gtk_widget_get_style_context(current_tool_button), "active");
    }
    if (btn){
        gtk_style_context_add_class(gtk_widget_get_style_context(btn), "active");
        current_tool_button = btn;
    }
}

//--------------tool buttons----------------
void 
on_tool_brush(GtkWidget* btn, gpointer){
    switch_tool(std::make_unique<Brush>(0xFF000000, 4));
    highlight_tool(btn);
}

void 
on_tool_eraser(GtkWidget* btn, gpointer){
    switch_tool(std::make_unique<Brush>(0xFFFFFFFF, 8));
    highlight_tool(btn);
}

void 
on_tool_fill(GtkWidget* btn, gpointer){
    switch_tool(std::make_unique<Fill>(current_theme->foreground));
    highlight_tool(btn);
}

//--------------undo/redo---------------
static void 
on_undo(GtkWidget*, gpointer){
    if (history.undo(*canvas)) gtk_widget_queue_draw(area);
    update_history_buttons();
}

static void 
on_redo(GtkWidget*, gpointer){
    if (history.redo(*canvas)) gtk_widget_queue_draw(area);
    update_history_buttons();
}

static void 
on_color_changed(GtkColorButton* btn, gpointer){
    if (!current_tool || !current_tool->usesColor()) return;

    GdkRGBA rgba;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(btn), &rgba);
    uint32_t color =
        ((uint8_t)(rgba.alpha * 255) << 24) |
        ((uint8_t)(rgba.red   * 255) << 16) |
        ((uint8_t)(rgba.green * 255) << 8) |
        ((uint8_t)(rgba.blue  * 255));
    current_tool->setColor(color);
}

static void 
on_size_changed(GtkRange* range, gpointer){
    if (!current_tool || !current_tool->supportsSize()) return;
    current_tool->setSize(static_cast<int>(gtk_range_get_value(range)));
}

void 
on_toggle_theme(GtkWidget*, gpointer){
    current_theme = (current_theme == &THEME_LIGHT) ? &THEME_DARK : &THEME_LIGHT;
    apply_theme_css(area, current_theme);
}

//---------------main--------------
int 
main(int argc, char** argv){
    gtk_init(&argc, &argv);
    

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Paint C++");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget* root = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* toolbar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    area = gtk_drawing_area_new();

    gtk_widget_set_hexpand(toolbar, FALSE);
    gtk_widget_set_size_request(toolbar, 60, -1);
    gtk_widget_set_hexpand(area, TRUE);
    gtk_widget_set_vexpand(area, TRUE);

    gtk_box_pack_start(GTK_BOX(root), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), area, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(window), root);

    canvas = std::make_unique<Canvas>(800, 600);
    canvas->clear(current_theme->background);

    apply_theme_css(window, current_theme);
    
    
    //---------------toolbar wdgets--------------
    size_slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 100, 1);
    gtk_scale_set_draw_value(GTK_SCALE(size_slider), TRUE);
    gtk_widget_set_sensitive(size_slider, FALSE);
    gtk_widget_set_hexpand(size_slider, TRUE);

    GtkWidget* btn_brush  = gtk_button_new_with_label("Brush");
    GtkWidget* btn_eraser = gtk_button_new_with_label("Eraser");
    GtkWidget* btn_fill   = gtk_button_new_with_label("Fill");
    btn_undo = gtk_button_new_with_label("Undo");
    btn_redo = gtk_button_new_with_label("Redo");
    GtkWidget* btn_save  = gtk_button_new_with_label("Save");
    GtkWidget* btn_theme = gtk_button_new_with_label("Theme");

    gtk_box_pack_start(GTK_BOX(toolbar), btn_brush,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_eraser, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_fill,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_undo,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_redo,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_save,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_theme,  FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(toolbar), size_slider, FALSE, FALSE, 4);

    GtkWidget* spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(spacer, TRUE);
    gtk_box_pack_start(GTK_BOX(toolbar), spacer, TRUE, TRUE, 0);

    color_button = gtk_color_button_new();
    gtk_widget_set_sensitive(color_button, FALSE);
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(color_button), TRUE);
    gtk_box_pack_start(GTK_BOX(toolbar), color_button, FALSE, FALSE, 6);

    gtk_widget_add_events(area,
        GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK |
        GDK_POINTER_MOTION_MASK);
    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);

    //---------------signals--------------
    g_signal_connect(area, "configure-event", G_CALLBACK(on_canvas_resize), nullptr);
    g_signal_connect(area, "draw", G_CALLBACK(on_draw), nullptr);
    g_signal_connect(area, "button-press-event", G_CALLBACK(on_button_press), nullptr);
    g_signal_connect(area, "button-release-event", G_CALLBACK(on_button_release), nullptr);
    g_signal_connect(area, "motion-notify-event", G_CALLBACK(on_motion), nullptr);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), nullptr);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    g_signal_connect(btn_brush,  "clicked", G_CALLBACK(on_tool_brush), btn_brush);
    g_signal_connect(btn_eraser, "clicked", G_CALLBACK(on_tool_eraser), btn_eraser);
    g_signal_connect(btn_fill,   "clicked", G_CALLBACK(on_tool_fill), btn_fill);
    g_signal_connect(btn_undo,   "clicked", G_CALLBACK(on_undo), area);
    g_signal_connect(btn_redo,   "clicked", G_CALLBACK(on_redo), area);
    g_signal_connect(btn_save,   "clicked", G_CALLBACK(on_save), window);
    g_signal_connect(btn_theme,  "clicked", G_CALLBACK(on_toggle_theme), window);

    g_signal_connect(color_button, "color-set", G_CALLBACK(on_color_changed), nullptr);
    g_signal_connect(size_slider, "value-changed", G_CALLBACK(on_size_changed), nullptr);

    update_color_button();
    update_size_slider();
    update_history_buttons();
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
