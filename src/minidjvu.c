/*
 * minidjvu.c - an example of using the library
 */

#include <minidjvu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/* TODO: remove duplicated code */


/* options */
int32 dpi = 300;
int32 pages_per_dict = 5; /* 0 means infinity */
int dpi_specified = 0;
int verbose = 0;
int smooth = 0;
int averaging = 0;
int match = 0;
int Match = 0;
int aggression = 100;
int erosion = 0;
int clean = 0;
int report = 0;
int no_prototypes = 0;
int warnings = 0;

/* Under Windows (MSVC), there is usually no strcasecmp.
 * So here's the rewrite.
 */
static int my_strcasecmp(const char *s1, const char *s2)
{
    int c1, c2;
    while(*s1)
    {
        int d;
        c1 = tolower(*s1++); c2 = tolower(*s2++);
        d = c1 - c2;
        if (d) return d;
    }
    return *s2;
}

static int ends_with_ignore_case(const char *s, const char *prefix)
{
    size_t sl = strlen(s);
    size_t pl = strlen(prefix);
    if (sl < pl) return 0;
    return !my_strcasecmp(s + sl - pl, prefix);
}

/* ========================================================================= */

/* file name template routines (for multipage encoding) {{{ */

static int number_of_digits(int n)
{
    int digits_count = 0;

    if (n == 0) return 1;

    while (n)
    {
        n /= 10;
        digits_count++;
    }

    return digits_count;
}

/*  len is the string's length.
 *  returns the number of trailing '0' characters
 */
static int number_of_trailing_zeroes(char *s, int len)
{
    int c = 0;
    int i = len - 1;
    while (i >= 0 && s[i] == '0')
    {
        i--;
        c++;
    }
    return c;
}

static void plant_number_into_template(int n, char *pattern, int len)
{
    int nlen = number_of_digits(n);
    char save;
    assert(n >= 0);
    assert(len >= nlen);
    save = pattern[len];
    sprintf(pattern + len - nlen, "%d", n);
    pattern[len] = save;
}

/* file name template routines (for multipage encoding) }}} */

/* ========================================================================= */

static void show_usage_and_exit(void)           /* {{{ */
{
    const char *what_it_does = "encode/decode bitonal DjVu files";
    if (strcmp(MDJVU_VERSION, mdjvu_get_version()))
    {
        printf("minidjvu - %s\n", what_it_does);
        printf("Warning: program and library version mismatch:\n");
        printf("    program version %s, library version %s.\n\n", MDJVU_VERSION, mdjvu_get_version());

    }
    else
    {
        printf("minidjvu %s - %s\n", MDJVU_VERSION, what_it_does);

    }
    printf("Usage:\n");
    printf("single page encoding/decoding:\n");
    printf("    minidjvu [options] <input file> <output file>\n");
    printf("multiple pages encoding (experimental):\n");
    printf("    minidjvu [options] <input file>... <djvu name template>\n");
    printf("    (the djvu name template looks like smth0000.djvu;\n");
    printf("     pages produced will start with smth0001.djvu)\n\n");
    printf("Formats supported:\n");

    printf("    DjVu (single-page bitonal), PBM, Windows BMP");
    if (mdjvu_have_tiff_support())
        printf(", TIFF.\n");
    else
        printf("; TIFF support is OFF.\n");

    printf("Options:\n");
    printf("    -A, --Averaging:               compute \"average\" representatives\n");
    printf("    -a <n>, --aggression <n>:      set aggression level (default 100)\n");
    printf("    -c, --clean                    remove small black pieces\n");
    printf("    -d <n> --dpi <n>:              set resolution in dots per inch\n");
    printf("    -e, --erosion                  sacrifice quality to gain in size\n");
    printf("    -l, --lossy:                   use all lossy options (-s -c -m -e -A)\n");
    printf("    -m, --match:                   match and substitute patterns\n");
    printf("    -n, --no-prototypes:           do not search for prototypes\n");
    printf("    -p <n>, --pages-per-dict <n>:  pages per dictionary (default all)\n");
    printf("    -r, --report:                  report multipage coding progress\n");
    printf("    -s, --smooth:                  remove some badly looking pixels\n");
    printf("    -v, --verbose:                 print messages about everything\n");
    printf("    -w, --warnings:                do not suppress TIFF warnings\n");
    printf("See the man page for detailed description of each option.\n");
    exit(2);
}                   /* }}} */

static int decide_if_bmp(const char *path)
{
    return ends_with_ignore_case(path, ".bmp");
}

static int decide_if_djvu(const char *path)
{
    return ends_with_ignore_case(path, ".djvu")
        || ends_with_ignore_case(path, ".djv");
}

static int decide_if_tiff(const char *path)
{
    return ends_with_ignore_case(path, ".tiff")
        || ends_with_ignore_case(path, ".tif");
}

/* ========================================================================= */

static mdjvu_image_t load_image(const char *path)
{
    mdjvu_error_t error;
    mdjvu_image_t image;

    if (verbose) printf("loading a DjVu page from `%s'\n", path);
    image = mdjvu_load_djvu_page(path, &error);
    if (!image)
    {
        fprintf(stderr, "%s: %s\n", path, mdjvu_get_error_message(error));
        exit(1);
    }
    if (verbose)
    {
        printf("loaded; the page has "MDJVU_INT32_FORMAT" bitmaps and "
               MDJVU_INT32_FORMAT" blits\n",
               mdjvu_image_get_bitmap_count(image),
               mdjvu_image_get_blit_count(image));
    }
    return image;
}

static mdjvu_matcher_options_t get_matcher_options(void)
{
    mdjvu_matcher_options_t m_options = NULL;
    if (match || Match)
    {
        m_options = mdjvu_matcher_options_create();
        mdjvu_use_matcher_method(m_options, MDJVU_MATCHER_PITH_2);
        if (Match)
            mdjvu_use_matcher_method(m_options, MDJVU_MATCHER_RAMPAGE);
        mdjvu_set_aggression(m_options, aggression);
    }
    return m_options;
}

static void sort_and_save_image(mdjvu_image_t image, const char *path)
{
    mdjvu_error_t error;

    mdjvu_compression_options_t options = mdjvu_compression_options_create();
    mdjvu_set_matcher_options(options, get_matcher_options());

    mdjvu_set_clean(options, clean);
    mdjvu_set_verbose(options, verbose);
    mdjvu_set_no_prototypes(options, no_prototypes);
    mdjvu_set_averaging(options, averaging);
    mdjvu_compress_image(image, options);
    mdjvu_compression_options_destroy(options);

    if (verbose) printf("encoding to `%s'\n", path);

    if (!mdjvu_save_djvu_page(image, path, NULL, &error, erosion))
    {
        fprintf(stderr, "%s: %s\n", path, mdjvu_get_error_message(error));
        exit(1);
    }
}

static mdjvu_bitmap_t load_bitmap(const char *path)
{
    mdjvu_error_t error;
    mdjvu_bitmap_t bitmap;

    if (decide_if_bmp(path))
    {
        if (verbose) printf("loading from Windows BMP file `%s'\n", path);
        bitmap = mdjvu_load_bmp(path, &error);
    }
    else if (decide_if_tiff(path))
    {
        if (verbose) printf("loading from TIFF file `%s'\n", path);
        if (!warnings)
            mdjvu_disable_tiff_warnings();
        if (dpi_specified)
            bitmap = mdjvu_load_tiff(path, NULL, &error);
        else
            bitmap = mdjvu_load_tiff(path, &dpi, &error);
        if (verbose) printf("resolution is "MDJVU_INT32_FORMAT" dpi\n", dpi);
    }
    else if (decide_if_djvu(path))
    {
        mdjvu_image_t image = load_image(path);
        bitmap = mdjvu_render(image);
        mdjvu_image_destroy(image);
        if (verbose)
        {
            printf("bitmap "MDJVU_INT32_FORMAT" x "MDJVU_INT32_FORMAT" rendered\n",
                   mdjvu_bitmap_get_width(bitmap),
                   mdjvu_bitmap_get_height(bitmap));
        }
    }
    else
    {
        if (verbose) printf("loading from PBM file `%s'\n", path);
        bitmap = mdjvu_load_pbm(path, &error);
    }

    if (!bitmap)
    {
        fprintf(stderr, "%s: %s\n", path, mdjvu_get_error_message(error));
        exit(1);
    }

    if (smooth)
    {
        if (verbose) printf("smoothing the bitmap\n");
        mdjvu_smooth(bitmap);
    }

    return bitmap;
}

static void save_bitmap(mdjvu_bitmap_t bitmap, const char *path)
{
    mdjvu_error_t error;
    int result;

    if (decide_if_bmp(path))
    {
        if (verbose) printf("saving to Windows BMP file `%s'\n", path);
        result = mdjvu_save_bmp(bitmap, path, dpi, &error);
    }
    else if (decide_if_tiff(path))
    {
        if (verbose) printf("saving to TIFF file `%s'\n", path);
        if (!warnings)
            mdjvu_disable_tiff_warnings();
        result = mdjvu_save_tiff(bitmap, path, &error);
    }
    else
    {
        if (verbose) printf("saving to PBM file `%s'\n", path);
        result = mdjvu_save_pbm(bitmap, path, &error);
    }

    if (!result)
    {
        fprintf(stderr, "%s: %s\n", path, mdjvu_get_error_message(error));
        exit(1);
    }
}

/* ========================================================================= */

static void decode(int argc, char **argv)
{
    mdjvu_image_t image;    /* a sequence of blits (what is stored in DjVu) */
    mdjvu_bitmap_t bitmap;  /* the result                                   */

    if (verbose) printf("\nDECODING\n");
    if (verbose) printf("________\n\n");

    image = load_image(argv[1]);
    bitmap = mdjvu_render(image);
    mdjvu_image_destroy(image);

    if (verbose)
    {
        printf("bitmap "MDJVU_INT32_FORMAT" x "MDJVU_INT32_FORMAT" rendered\n",
               mdjvu_bitmap_get_width(bitmap),
               mdjvu_bitmap_get_height(bitmap));
    }

    if (smooth)
    {
        if (verbose) printf("smoothing the bitmap\n");
        mdjvu_smooth(bitmap);
    }

    save_bitmap(bitmap, argv[2]);
    mdjvu_bitmap_destroy(bitmap);
}


static mdjvu_image_t split_and_destroy(mdjvu_bitmap_t bitmap)
{
    mdjvu_image_t image;
    if (verbose) printf("splitting the bitmap into pieces\n");
    image = mdjvu_split(bitmap, dpi, /* options:*/ NULL);
    mdjvu_bitmap_destroy(bitmap);
    if (verbose)
    {
        printf("the splitted image has "MDJVU_INT32_FORMAT" pieces\n",
                mdjvu_image_get_blit_count(image));
    }
    if (clean)
    {
        if (verbose) printf("cleaning\n");
        mdjvu_clean(image);
        if (verbose)
        {
            printf("the cleaned image has "MDJVU_INT32_FORMAT" pieces\n",
                    mdjvu_image_get_blit_count(image));
        }
    }
    return image;
}


static void encode(int argc, char **argv)
{
    mdjvu_bitmap_t bitmap;
    mdjvu_image_t image;

    if (verbose) printf("\nENCODING\n");
    if (verbose) printf("________\n\n");

    bitmap = load_bitmap(argv[1]);

    image = split_and_destroy(bitmap);
    sort_and_save_image(image, argv[2]);
    mdjvu_image_destroy(image);
}


/* Filtering is nondjvu->nondjvu job. */
static void filter(int argc, char **argv)
{
    mdjvu_bitmap_t bitmap;

    if (verbose) printf("\nFILTERING\n");
    if (verbose) printf("_________\n\n");

    bitmap = load_bitmap(argv[1]);
    save_bitmap(bitmap, argv[2]);
    mdjvu_bitmap_destroy(bitmap);
}


static const char *strip(const char *str, char sep)
{
    const char *t = strrchr(str, sep);
    if (t)
        return t + 1;
    else
        return str;
}

/* return path without a directory name */ 
static const char *strip_dir(const char *path)
{
    return strip(strip(path, '\\'), '/');
}


static void multipage_encode(int n, char **pages, char *pattern)
{
    mdjvu_image_t *images;
    mdjvu_image_t dict;
    int i;
    int pattern_len;
    char *dict_template;
    const char *dict_suffix = ".iff";
    const int dict_suffix_size = (int) (strlen(dict_suffix) + 1);
    mdjvu_compression_options_t options;
    mdjvu_bitmap_t bitmap;
    mdjvu_error_t error;
    int32 pages_compressed;

    if (ends_with_ignore_case(pattern, ".djv"))
        pattern_len = (int) (strlen(pattern) - 4);
    else if (ends_with_ignore_case(pattern, ".djvu"))
        pattern_len = (int) (strlen(pattern) - 5);
    else
    {
        fprintf(stderr, "when encoding many pages, output file must be DjVu\n");
        exit(1);
    }

    dict_template = MDJVU_MALLOCV(char, pattern_len + dict_suffix_size);
    strncpy(dict_template, pattern, pattern_len);
    strcpy(dict_template + pattern_len, dict_suffix);

    if (number_of_trailing_zeroes(pattern, pattern_len) < number_of_digits(n))
    {
        fprintf(stderr, "template should end with sufficient (%d in this case) number of zeroes, like in `foo000.djvu'\n",
                number_of_digits(n));
        exit(1);
    }

    if (verbose) printf("\nMULTIPAGE ENCODING\n");
    if (verbose) printf("__________________\n\n");
    if (verbose) printf("dictionaries will be saved in files with template `%s'\n", dict_template);
    if (verbose) printf("%d pages total\n", n);

    options = mdjvu_compression_options_create();
    mdjvu_set_matcher_options(options, get_matcher_options());

    mdjvu_set_clean(options, clean);
    mdjvu_set_verbose(options, verbose);
    mdjvu_set_no_prototypes(options, no_prototypes);
    mdjvu_set_report(options, report);
    mdjvu_set_averaging(options, averaging);
    mdjvu_set_report_total_pages(options, n);

    /* compressing */
    if (pages_per_dict <= 0) pages_per_dict = n;
    if (pages_per_dict > n) pages_per_dict = n;
    images = MDJVU_MALLOCV(mdjvu_image_t, pages_per_dict);
    pages_compressed = 0;

    while (n - pages_compressed)
    {
        int32 pages_to_compress = n - pages_compressed;
        if (pages_to_compress > pages_per_dict)
            pages_to_compress = pages_per_dict;

        mdjvu_set_report_start_page(options, pages_compressed + 1);

        for (i = 0; i < pages_to_compress; i++)
        {
            bitmap = load_bitmap(pages[pages_compressed + i]);
            images[i] = split_and_destroy(bitmap);
            if (report)
                printf("Loading: %d of %d completed\n", pages_compressed + i + 1, n);
        }

        dict = mdjvu_compress_multipage(pages_to_compress, images, options);

        plant_number_into_template(pages_compressed + 1, dict_template, pattern_len);
        if (!mdjvu_save_djvu_dictionary(dict, dict_template, &error, erosion))
        {
            fprintf(stderr, "%s: %s\n", dict_template, mdjvu_get_error_message(error));
            exit(1);
        }

        for (i = 0; i < pages_to_compress; i++)
        {
            plant_number_into_template(pages_compressed + i + 1, pattern, pattern_len);
            if (verbose) printf("saving page #%d into %s using dictionary %s\n", pages_compressed + i + 1, pattern, dict_template);
            if (!mdjvu_save_djvu_page(images[i], pattern, strip_dir(dict_template), &error, erosion))
            {
                fprintf(stderr, "%s: %s\n", pattern, mdjvu_get_error_message(error));
                exit(1);
            }
            mdjvu_image_destroy(images[i]);
            if (report)
                printf("Saving: %d of %d completed\n", pages_compressed + i + 1, n);
        }
        mdjvu_image_destroy(dict);
        pages_compressed += pages_to_compress;
    }

    /* destroying */
    mdjvu_compression_options_destroy(options);

    MDJVU_FREEV(images);
    MDJVU_FREEV(dict_template);
}

/* same_option(foo, "opt") returns 1 in three cases:
 *
 *      foo is "o" (first letter of opt)
 *      foo is "opt"
 *      foo is "-opt"
 */
static int same_option(const char *option, const char *s)
{
    if (option[0] == s[0] && !option[1]) return 1;
    if (!strcmp(option, s)) return 1;
    if (option[0] == '-' && !strcmp(option + 1, s)) return 1;
    return 0;
}

static int process_options(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc && argv[i][0] == '-'; i++)
    {
        char *option = argv[i] + 1;
        if (same_option(option, "verbose"))
            verbose = 1;
        else if (same_option(option, "smooth"))
            smooth = 1;
        else if (same_option(option, "match"))
            match = 1;
        else if (same_option(option, "Match"))
            Match = 1;
        else if (same_option(option, "no-prototypes"))
            no_prototypes = 1;
        else if (same_option(option, "erosion"))
            erosion = 1;
        else if (same_option(option, "clean"))
            clean = 1;
        else if (same_option(option, "warnings"))
            warnings = 1;
        else if (same_option(option, "report"))
            report = 1;
        else if (same_option(option, "Averaging"))
            averaging = 1;
        else if (same_option(option, "lossy"))
        {
            smooth = 1;
            match = 1;
            erosion = 1;
            clean = 1;
            averaging = 1;
        }
        else if (same_option(option, "Lossy"))
        {
            smooth = 1;
            Match = match = 1;
            erosion = 1;
            clean = 1;
            averaging = 1;
        }
        else if (same_option(option, "pages-per-dict"))
        {
            i++;
            if (i == argc) show_usage_and_exit();
            pages_per_dict = atoi(argv[i]);
            if (pages_per_dict <= 0)
            {
                fprintf(stderr, "bad --pages-per-dict value\n");
                exit(2);
            }
        }
        else if (same_option(option, "dpi"))
        {
            i++;
            if (i == argc) show_usage_and_exit();
            dpi = atoi(argv[i]);
            dpi_specified = 1;
            if (dpi < 20 || dpi > 2000)
            {
                fprintf(stderr, "bad resolution\n");
                exit(2);
            }
        }
        else if (same_option(option, "aggression"))
        {
            i++;
            if (i == argc) show_usage_and_exit();
            aggression = atoi(argv[i]);
            match = 1;
        }
        else
        {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            exit(2);
        }
    }
    return i;
}

int main(int argc, char **argv)
{
    int arg_start;
    const char *sanity_error_message;

    /* check sizeof(int32) == 4 and such gibberish */
    sanity_error_message = mdjvu_check_sanity();
    if (sanity_error_message)
    {
        fprintf(stderr, "%s\n", sanity_error_message);
        exit(1);
    }

    arg_start = process_options(argc, argv);

    argc -= arg_start - 1;
    argv += arg_start - 1;

    if (argc < 3)
        show_usage_and_exit();

    if (argc > 3)
        multipage_encode(argc - 2, argv + 1, argv[argc - 1]);
    else if (decide_if_djvu(argv[2]))
    {
        encode(argc, argv);
    }
    else
    {
        if (decide_if_djvu(argv[1]))
            decode(argc, argv);
        else
            filter(argc, argv);
    }

    if (verbose) printf("\n");
    #ifndef NDEBUG 
        if (alive_bitmap_counter)
           printf("alive_bitmap_counter = %d\n", alive_bitmap_counter);
    #endif
    return 0;
}
