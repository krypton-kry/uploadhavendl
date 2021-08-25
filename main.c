#include "utils.h"
#include <lexbor/html/html.h>
#include <lexbor/css/css.h>
#include <lexbor/selectors/selectors.h>
#include <unistd.h>

lxb_status_t
find_callback(lxb_dom_node_t *node, lxb_css_selector_specificity_t *spec,
              void *ctx)
{
	lexbor_str_t test = {0};
	lxb_html_serialize_str(node, &test);

	lxb_status_t status;
	lxb_html_document_t *document;
	lxb_dom_collection_t *collection;
	lxb_dom_element_t *element;

	document = lxb_html_document_create();
	status = lxb_html_document_parse(document, test.data, test.length);;
	if (status != LXB_STATUS_OK) {
		return EXIT_FAILURE;
	}

	collection = lxb_dom_collection_make(&document->dom_document, 16);
	element = lxb_dom_interface_element(lxb_html_document_body_element(document));

	status = lxb_dom_elements_by_tag_name(element, collection,
	                                      (const lxb_char_t *) "a", 1);

	if (status != LXB_STATUS_OK || lxb_dom_collection_length(collection) == 0) {
		puts("Failed to find element");
	}

	element = lxb_dom_collection_element(collection, 0);
	const lxb_char_t *value;
	size_t value_len;
	const lxb_char_t name[] = "href";
	size_t name_size = sizeof(name) - 1;

	/* Get value by qualified name */
	value = lxb_dom_element_get_attribute(element, name, name_size, &value_len);
	if (value == NULL) {
		puts("Failed to get attribute value by qualified name");
	}

	printf("\033[0;32mDirect link : \033[0m%s\n", value);

	/* Destroy Collection */
	lxb_dom_collection_destroy(collection, true);

	/* Destroy HTML Document. */
	(void) lxb_html_document_destroy(document);

	return LXB_STATUS_OK;
}

int main(int argc, char* argv[])
{

	if ( argc < 2) {
		printf("Usage : %s url\n", argv[0]);
		return -1;
	}

	/* copy and convert to lower */
	char *p = strdup(argv[1]);
	to_lower(p);

	if ( !starts_with("https://uploadhaven.com", p) && !starts_with("http://uploadhaven.com", p)) {
		printf("\033[0;31mUploadhaven url not found!  : \033[0m%s\n", argv[1]);
		printf("Usage : %s url\n", argv[0]);
		return -2;
	}

	free(p);

	CURL *curl;
	struct MemStr chunk;

	chunk.memory = malloc(1);
	chunk.size = 0;

	curl = curl_easy_init();

	INFO("Get Page : %s", argv[1]);
	curl_get(curl, argv[1], &chunk, NULL);

	INFO("Processing Page : %s", argv[1]);

	lxb_status_t status;
	lxb_html_document_t *document;
	lxb_dom_collection_t *collection;
	lxb_dom_element_t *element;
	lxb_dom_attr_t *attr;

	struct input inputs[4];
	/* HTML Data. */

	lxb_char_t html[chunk.size];
	strcpy((char *)html, chunk.memory);

	/* Create HTML Document. */

	document = lxb_html_document_create();
	status = lxb_html_document_parse(document, html,
	                                 sizeof(html) / sizeof(lxb_char_t) - 1);;
	if (status != LXB_STATUS_OK) {
		return EXIT_FAILURE;
	}

	collection = lxb_dom_collection_make(&document->dom_document, 16);
	element = lxb_dom_interface_element(lxb_html_document_body_element(document));

	status = lxb_dom_elements_by_tag_name(element, collection,
	                                      (const lxb_char_t *) "input", 5);

	if (status != LXB_STATUS_OK || lxb_dom_collection_length(collection) == 0) {
		puts("Failed to find element");
	}

	element = lxb_dom_collection_element(collection, 0);

	/* Loop through elements and then get all attributes when not null*/
	int i = 0;
	while (element != NULL) {
		const lxb_char_t *tmp, *tmp2;
		size_t tmp_len, tmp2_len;

		attr = lxb_dom_element_first_attribute(element);

		while (attr != NULL) {
			tmp = lxb_dom_attr_qualified_name(attr, &tmp_len);
			tmp2 = lxb_dom_attr_value(attr, &tmp2_len);

			if (tmp2 != NULL) {

				if (!strcmp((const char*)tmp, "name")) {
					inputs[i].name = (char *)tmp2;
				}

				if (!strcmp((const char*)tmp, "value")) {
					inputs[i].value = (char *)tmp2;
				}
			}

			attr = lxb_dom_element_next_attribute(attr);
		}

		i++;
		element = lxb_dom_collection_element(collection, i);
	}


	char *test[2048];

	for ( unsigned int i = 0; i < sizeof(inputs) / sizeof(struct input) ; i++)
	{
		strcat((char *)test, "&");
		strcat((char *)test, inputs[i].name);
		strcat((char *)test, "=");
		strcat((char *)test, inputs[i].value);
	}

	/* do post request*/
	INFO("Sleeping for 5 seconds ... ");
	sleep(5);
	struct MemStr chunk2;

	chunk2.memory = malloc(1);
	chunk2.size = 0;

	curl_get(curl, argv[1], &chunk2, (char *)test);

	lxb_status_t status1;
	lxb_dom_node_t *body;
	lxb_selectors_t *selectors;
	lxb_css_selectors_t *css_selectors;
	lxb_html_document_t *document1;
	lxb_css_parser_t *parser;
	lxb_css_selector_list_t *list_one;

	static const lxb_char_t slctrs_one[] = ".download-timer > div > a";


	/* Create HTML Document. */
	lxb_char_t ht [chunk2.size];
	strcpy((char*)ht, chunk2.memory);
	document1 = lxb_html_document_create();
	status1 = lxb_html_document_parse(document1, ht,
	                                  sizeof(ht) / sizeof(lxb_char_t) - 1);
	if (status1 != LXB_STATUS_OK) {
		return EXIT_FAILURE;
	}

	body = lxb_dom_interface_node(lxb_html_document_body_element(document1));

	/* Create CSS parser. */

	parser = lxb_css_parser_create();
	status1 = lxb_css_parser_init(parser, NULL, NULL);
	if (status1 != LXB_STATUS_OK) {
		return EXIT_FAILURE;
	}

	/* Create CSS Selector parser. */

	css_selectors = lxb_css_selectors_create();
	status1 = lxb_css_selectors_init(css_selectors, 32);
	if (status1 != LXB_STATUS_OK) {
		return EXIT_FAILURE;
	}

	/* It is important that a new selector object is not created internally
	 * for each call to the parser.
	 */
	lxb_css_parser_selectors_set(parser, css_selectors);

	/* Selectors. */

	selectors = lxb_selectors_create();
	status1 = lxb_selectors_init(selectors);
	if (status1 != LXB_STATUS_OK) {
		return EXIT_FAILURE;
	}

	/* Parse and get the log. */

	list_one = lxb_css_selectors_parse(parser, slctrs_one,
	                                   sizeof(slctrs_one) / sizeof(lxb_char_t) - 1);
	if (list_one == NULL) {
		return EXIT_FAILURE;
	}


	/* Find HTML nodes by CSS Selectors. */
	status1 = lxb_selectors_find(selectors, body, list_one,
	                             find_callback, NULL);
	if (status1 != LXB_STATUS_OK) {
		return EXIT_FAILURE;
	}
	/* Destroy Selectors object. */
	(void) lxb_selectors_destroy(selectors, true);

	/* Destroy resources for CSS Parser. */
	(void) lxb_css_parser_destroy(parser, true);

	/* Destroy CSS Selectors List memory. */
	(void) lxb_css_selectors_destroy(css_selectors, true, true);
	/* or use */
	/* lxb_css_selector_list_destroy_memory(list_one); */

	/* Destroy Collection */
	lxb_dom_collection_destroy(collection, true);

	/* Destroy HTML Document. */
	(void) lxb_html_document_destroy(document);
	(void) lxb_html_document_destroy(document1);

	/* cleanup */
	free(chunk.memory);
	free(chunk2.memory);
	curl_easy_cleanup(curl);
	return 0;
}
