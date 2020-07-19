#include <module.h>
#include <dryos.h>
#include <menu.h>
#include <config.h>
#include <math.h>

static float units[] = {10, 25.4, 304.8};
#define UNITS_TEXT "cm", "inch", "feet"

#define MAX_FOCAL_LENGTH_OPTIONS 10
#define MAX_FOCAL_LENGTH_TEXT 7

static int focal_length_options[MAX_FOCAL_LENGTH_OPTIONS] = {0,0,0,0,0,0,0,0,0,0};

struct focal_length_entry
{
    struct menu_entry* menu_entry;
    char name[MAX_FOCAL_LENGTH_TEXT];
};

static struct focal_length_entry* focal_length_entries = 0;
static int new_focal_length = 1;

static CONFIG_ARRAY_ELEMENT("tilt.calculator.focal_length_options.0", focal_length_options, 0, 0);
static CONFIG_ARRAY_ELEMENT("tilt.calculator.focal_length_options.1", focal_length_options, 1, 0);
static CONFIG_ARRAY_ELEMENT("tilt.calculator.focal_length_options.2", focal_length_options, 2, 0);
static CONFIG_ARRAY_ELEMENT("tilt.calculator.focal_length_options.3", focal_length_options, 3, 0);
static CONFIG_ARRAY_ELEMENT("tilt.calculator.focal_length_options.4", focal_length_options, 4, 0);
static CONFIG_ARRAY_ELEMENT("tilt.calculator.focal_length_options.5", focal_length_options, 5, 0);
static CONFIG_ARRAY_ELEMENT("tilt.calculator.focal_length_options.6", focal_length_options, 6, 0);
static CONFIG_ARRAY_ELEMENT("tilt.calculator.focal_length_options.7", focal_length_options, 7, 0);
static CONFIG_ARRAY_ELEMENT("tilt.calculator.focal_length_options.8", focal_length_options, 8, 0);
static CONFIG_ARRAY_ELEMENT("tilt.calculator.focal_length_options.9", focal_length_options, 9, 0);

static CONFIG_INT("tilt.calculator.focallength", focal_length, 24);
static CONFIG_INT("tilt.calculator.unit", unit_sel, 1);
static CONFIG_INT("tilt.calculator.distance", distance_sel, 60);

static MENU_SELECT_FUNC(select_focal_length)
{
    focal_length = *((int*)priv);
    menu_close_submenu();
}

static MENU_UPDATE_FUNC(clear_right)
{
    MENU_SET_VALUE("");
}

static void sort_focal_lengths()
{
    int i = 0;
    int j = 0;
    
    for (i=0; i<MAX_FOCAL_LENGTH_OPTIONS; i++)
    {
        for (j=i+1; j<MAX_FOCAL_LENGTH_OPTIONS; j++)
        {
            if (focal_length_options[i] - focal_length_options[j] > 0)
            {
                int aux = focal_length_options[i];
                focal_length_options[i] = focal_length_options[j];
                focal_length_options[j] = aux;
            }
        }
    }
}

static void clear_menu() {
    int focal_length_index = 0;
    int menu_index = 0;

    //how many options do we have
    for (focal_length_index = 0; focal_length_index < MAX_FOCAL_LENGTH_OPTIONS; focal_length_index++)
    {
        if (focal_length_options[focal_length_index] != 0) 
        {
            menu_remove("Focal Length", focal_length_entries[menu_index].menu_entry, 1);
            menu_index++;
        }
    }
    free(focal_length_entries);
    focal_length_entries = 0;
}

static void build_menu() {
    int focal_length_count = 0;
    int focal_length_index = 0;

    //how many options do we have
    for (focal_length_index = 0; focal_length_index < MAX_FOCAL_LENGTH_OPTIONS; focal_length_index++)
    {
        if (focal_length_options[focal_length_index] != 0) 
        {
            focal_length_count++ ;
        }  
    }

    //if we have nothing, reset to default
    if (focal_length_count == 0)
    {
        focal_length_count = 4;
        focal_length_options[6] = 17;
        focal_length_options[7] = 24;
        focal_length_options[8] = 45;
        focal_length_options[9] = 90;
        focal_length = 17;
    }

    //create our workspace and clear the memory
    focal_length_entries = malloc(focal_length_count * sizeof(struct focal_length_entry));
    memset(focal_length_entries, 0, focal_length_count * sizeof(struct focal_length_entry));

    //create the menus
    struct menu_entry* focal_length_menu = malloc(focal_length_count * sizeof(struct menu_entry));

    int menu_index = 0;

    for (focal_length_index = 0; focal_length_index < MAX_FOCAL_LENGTH_OPTIONS; focal_length_index++)
    {
        if (focal_length_options[focal_length_index] != 0) 
        {
            //save the selection
            snprintf(focal_length_entries[menu_index].name, MAX_FOCAL_LENGTH_TEXT,"%d mm", focal_length_options[focal_length_index]);

            //create menu and zero memory
            focal_length_entries[menu_index].menu_entry = malloc(sizeof(struct menu_entry));
            memset(focal_length_entries[menu_index].menu_entry, 0, sizeof(struct menu_entry));

            //value all the interesting stuff on the menu
            focal_length_entries[menu_index].menu_entry->name = focal_length_entries[menu_index].name;
            focal_length_entries[menu_index].menu_entry->select = select_focal_length;
            focal_length_entries[menu_index].menu_entry->priv = &focal_length_options[focal_length_index];
            focal_length_entries[menu_index].menu_entry->icon_type = IT_ACTION;
            focal_length_entries[menu_index].menu_entry->update = clear_right;

            //copy the temporary menu struct to the real thing
            memcpy(&focal_length_menu[menu_index], focal_length_entries[menu_index].menu_entry, sizeof(struct menu_entry));

            //release the temp memory
            free(focal_length_entries[menu_index].menu_entry);

            //reset our pointer
            focal_length_entries[menu_index].menu_entry = &focal_length_menu[menu_index];

            menu_index++;
        }
    }

    menu_add("Focal Length", focal_length_menu, focal_length_count);

}

static MENU_UPDATE_FUNC(degrees_update)
{
	double height = distance_sel * units[unit_sel];	
	int degrees = roundf(1800.0f / M_PI * asinf(focal_length/height));
     
	MENU_SET_VALUE("%d.%d degrees", degrees/10, degrees%10);
}

static MENU_UPDATE_FUNC(focal_length_update)
{
	MENU_SET_VALUE("%d mm", focal_length);
}

static MENU_SELECT_FUNC(add_focal_length)
{
    int focal_length_index = 0;
 
    //how many options do we have, and where should we insert
    for (focal_length_index = 0; focal_length_index < MAX_FOCAL_LENGTH_OPTIONS; focal_length_index++)
    {
        //if it's already in our list, we're done
        if (focal_length_options[focal_length_index] == new_focal_length) 
        {
            return;
        }
    }

    //if we're all full then we're done
    if (focal_length_options[0] != 0) 
    {
        return;
    } 
    
    clear_menu();

    //if we have space, insert new value
    focal_length_options[0] = new_focal_length;
    focal_length = new_focal_length;

    sort_focal_lengths();

    build_menu();
    menu_close_submenu();
}


static MENU_SELECT_FUNC(remove_focal_length)
{
    int focal_length_index = 0;

    clear_menu();
 
    //how many options do we have, and where should we insert
    for (focal_length_index = 0; focal_length_index < MAX_FOCAL_LENGTH_OPTIONS; focal_length_index++)
    {
        //find it and clear it
        if (focal_length_options[focal_length_index] == focal_length) 
        {
            focal_length_options[focal_length_index] = 0;
        }
    }

    sort_focal_lengths();

    //pick the first one off the list
    for (focal_length_index = 0; focal_length_index < MAX_FOCAL_LENGTH_OPTIONS; focal_length_index++)
    {
        if (focal_length_options[focal_length_index] != 0) 
        {
            focal_length = focal_length_options[focal_length_index];
            break;
        }  
    }

    build_menu();
}


static struct menu_entry tilt_calc_menu[] =
{
    {
        .name = "Tilt Calculator",
        .select = menu_open_submenu,
        .submenu_width = 710,
        .children =  (struct menu_entry[]) 
         {
            {
                .name = "Focal Length",
                .select = menu_open_submenu,
                .update = focal_length_update,
                .help = "Select the lens focal length.",
                .children =  (struct menu_entry[]) {
                    MENU_EOL,
                }
            },
            {
                .name = "Unit",
                .priv = &unit_sel,
                .min = 0,
                .max = COUNT(units)-1,
                .choices = CHOICES(UNITS_TEXT),
                .help = "Select the unit for distance to hinge point.",
            },
            {
                .name = "Distance",
                .priv = &distance_sel,
                .min = 1,
                .max = 500,
                .help = "Select the distance to hinge point.",
            },
            {
                .name = "Tilt degrees",
                .update = degrees_update,
                .icon_type = IT_ALWAYS_ON,
                .help  = "[READ-ONLY] Set your tilt to this value.",
            },
             {
                .name = "Add Focal Length",
                .select = menu_open_submenu,
                .children =  (struct menu_entry[]) {
                    {
                        .name = "New Focal Length",
                        .min = 1,
                        .max = 200,
                        .priv = &new_focal_length,
                        .help = "Select the new focal length.",
                    },
                    {
                        .name = "OK",
                        .select = add_focal_length,
                        .help = "Add the new focal length to the short list.",
                    },
                    MENU_EOL,
                },
                .help = "Add a new focal length to the short list.",
            },
           {
                .name = "Remove Focal Length",
                .select = remove_focal_length,
                .icon_type = IT_ACTION,
                .help = "Remove the currently selected focal length from the short list.",
            },
            MENU_EOL,
        }
    }
};

static unsigned int tilt_calc_init()
{
    menu_add("Debug", tilt_calc_menu, COUNT(tilt_calc_menu));
    build_menu();
    return 0;
}

static unsigned int tilt_calc_deinit()
{
    clear_menu();
	menu_remove("Debug", tilt_calc_menu, COUNT(tilt_calc_menu));
	return 0;
}

MODULE_INFO_START()
    MODULE_INIT(tilt_calc_init)
    MODULE_DEINIT(tilt_calc_deinit)
MODULE_INFO_END()

MODULE_CONFIGS_START()
    MODULE_CONFIG(focal_length)
    MODULE_CONFIG(unit_sel)
    MODULE_CONFIG(distance_sel)
    MODULE_CONFIG(focal_length_options0)
    MODULE_CONFIG(focal_length_options1)
    MODULE_CONFIG(focal_length_options2)
    MODULE_CONFIG(focal_length_options3)
    MODULE_CONFIG(focal_length_options4)
    MODULE_CONFIG(focal_length_options5)
    MODULE_CONFIG(focal_length_options6)
    MODULE_CONFIG(focal_length_options7)
    MODULE_CONFIG(focal_length_options8)
    MODULE_CONFIG(focal_length_options9)
MODULE_CONFIGS_END()