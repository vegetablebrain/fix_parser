/* @file   fix_parser_tests.c
   @author Dmitry S. Melnikov, dmitryme@gmail.com
   @date   Created on: 08/03/2012 02:41:02 PM
*/

#include <check.h>

#include <fix_parser.h>
#include <fix_parser_priv.h>

START_TEST(CreateParserTest)
{
   FIXParserAttrs attrs = {512, 0, 2, 5, 2, 5};
   FIXParser* parser = fix_parser_create("fix_descr/fix.4.4.xml", &attrs, PARSER_FLAG_CHECK_ALL);
   fail_unless(parser != NULL);
   fail_unless(parser->error.code == 0);
   fail_unless(parser->flags == PARSER_FLAG_CHECK_ALL);
   fail_unless(parser->page != NULL);
   fail_unless(parser->page->next != NULL);
   fail_unless(parser->page->next->next == NULL);
   fail_unless(parser->used_pages == 0);
   fail_unless(parser->attrs.maxPages == 5);
   fail_unless(parser->attrs.pageSize == 512);
   fail_unless(parser->group != NULL);
   fail_unless(parser->used_groups == 0);
   fail_unless(parser->attrs.maxGroups == 5);

   fix_parser_free(parser);
}
END_TEST

START_TEST(SetErrorParserTest)
{
   FIXParserAttrs attrs = {512, 0, 2, 5, 2, 5};
   FIXParser* parser = fix_parser_create("fix_descr/fix.4.4.xml", &attrs, PARSER_FLAG_CHECK_ALL);
   fail_unless(parser != NULL);

   fix_error_set(&parser->error, FIX_ERROR_NO_MORE_PAGES, "No more pages available");

   fail_unless(parser->error.code == FIX_ERROR_NO_MORE_PAGES);
   fail_unless(!strcmp(parser->error.text, "No more pages available"));

   fix_error_reset(&parser->error);

   fail_unless(parser->error.code == 0);
   fail_unless(!strcmp(parser->error.text, ""));

   fix_parser_free(parser);
}
END_TEST

START_TEST(GetFreePageTest)
{
   FIXParserAttrs attrs = {512, 0, 2, 0, 2, 0};
   FIXParser* parser = fix_parser_create("fix_descr/fix.4.4.xml", &attrs, PARSER_FLAG_CHECK_ALL);
   fail_unless(parser != NULL);
   fail_unless(parser->error.code == 0);

   FIXPage* nextp = parser->page->next;
   FIXPage* p = fix_parser_alloc_page(parser, 0);

   fail_unless(p->next == NULL);
   fail_unless(parser->page == nextp);
   fail_unless(parser->used_pages == 1);

   FIXPage* p1 = fix_parser_alloc_page(parser, 0);
   fail_unless(p1 == nextp);
   fail_unless(p1->next == NULL);
   fail_unless(parser->used_pages == 2);

   FIXPage* p2 = fix_parser_alloc_page(parser, 0);
   fail_unless(parser->page == NULL);
   fail_unless(p2 != nextp);
   fail_unless(p1->next == NULL);
   fail_unless(parser->used_pages == 3);

   fix_parser_free_page(parser, p);

   fail_unless(parser->page == p);

   fix_parser_free_page(parser, p2);
   fail_unless(parser->page == p2);
   fail_unless(parser->page->next == p);

   fix_parser_free_page(parser, p1);
   fail_unless(parser->page == p1);
   fail_unless(parser->page->next == p2);

   fail_unless(parser->used_pages == 0);

   fix_parser_free(parser);
}
END_TEST

START_TEST(MaxPagesTest)
{
   FIXParserAttrs attrs = {512, 0, 2, 2, 2, 0};
   FIXParser* parser = fix_parser_create("fix_descr/fix.4.4.xml", &attrs, PARSER_FLAG_CHECK_ALL);
   fail_unless(parser != NULL);
   fail_unless(parser->error.code == 0);

   FIXPage* nextp = parser->page->next;
   fail_unless(parser->page != NULL);

   FIXPage* p = fix_parser_alloc_page(parser, 0);
   fail_unless(p->next == NULL);
   fail_unless(parser->page == nextp);
   fail_unless(parser->used_pages == 1);

   FIXPage* p1 = fix_parser_alloc_page(parser, 0);
   fail_unless(p1 == nextp);
   fail_unless(p1->next == NULL);
   fail_unless(parser->used_pages == 2);

   FIXPage* p2 = fix_parser_alloc_page(parser, 0);
   fail_unless(p2 == NULL);
   fail_unless(parser->error.code == FIX_ERROR_NO_MORE_PAGES);
   fail_unless(parser->page == NULL);
   fail_unless(parser->used_pages == 2);

   fix_parser_free_page(parser, p);

   fail_unless(parser->page == p);
   fail_unless(parser->used_pages == 1);

   p = fix_parser_alloc_page(parser, 0);

   fail_unless(p->next == NULL);
   fail_unless(parser->page == NULL);
   fail_unless(parser->used_pages == 2);

   fix_parser_free(parser);
}
END_TEST

START_TEST(MaxGroupsTest)
{
   FIXParserAttrs attrs = {512, 0, 2, 0, 2, 2};
   FIXParser* parser = fix_parser_create("fix_descr/fix.4.4.xml", &attrs, PARSER_FLAG_CHECK_ALL);
   fail_unless(parser != NULL);
   fail_unless(parser->error.code == 0);

   FIXGroup* nextg = parser->group->next;
   fail_unless(parser->group != NULL);

   FIXGroup* g = fix_parser_alloc_group(parser);
   fail_unless(g->next == NULL);
   fail_unless(parser->group == nextg);
   fail_unless(parser->used_groups == 1);

   FIXGroup* g1 = fix_parser_alloc_group(parser);
   fail_unless(g1 == nextg);
   fail_unless(g1->next == NULL);
   fail_unless(parser->used_groups == 2);

   FIXGroup* p2 = fix_parser_alloc_group(parser);
   fail_unless(p2 == NULL);
   fail_unless(parser->error.code == FIX_ERROR_NO_MORE_GROUPS);
   fail_unless(parser->group == NULL);
   fail_unless(parser->used_groups == 2);

   fix_parser_free_group(parser, g);

   g = fix_parser_alloc_group(parser);
   fail_unless(g->next == NULL);
   fail_unless(parser->group == NULL);
   fail_unless(parser->used_groups == 2);

   fix_parser_free(parser);
}
END_TEST

START_TEST(MaxPageSizeTest)
{
   {
      FIXParserAttrs attrs = {512, 0, 1, 0, 2, 0};
      FIXParser* parser = fix_parser_create("fix_descr/fix.4.4.xml", &attrs, PARSER_FLAG_CHECK_ALL);
      fail_unless(parser != NULL);
      fail_unless(parser->error.code == 0);

      FIXPage* p = fix_parser_alloc_page(parser, 0);
      fail_unless(p->next == NULL);
      fail_unless(parser->used_pages == 1);
      fail_unless(p->size == 512);

      FIXPage* p1 = fix_parser_alloc_page(parser, 1024);
      fail_unless(p1->next == NULL);
      fail_unless(parser->used_pages == 2);
      fail_unless(p1->size == 1024);

      FIXPage* p2 = fix_parser_alloc_page(parser, 10240);
      fail_unless(p2->next == NULL);
      fail_unless(parser->used_pages == 3);
      fail_unless(p2->size == 10240);

      fix_parser_free(parser);
   }
   {
      FIXParserAttrs attrs = {512, 512, 1, 0, 2, 0};
      FIXParser* parser = fix_parser_create("fix_descr/fix.4.4.xml", &attrs, PARSER_FLAG_CHECK_ALL);
      fail_unless(parser != NULL);
      fail_unless(parser->error.code == 0);

      FIXPage* p = fix_parser_alloc_page(parser, 0);
      fail_unless(p != NULL);
      fail_unless(parser->used_pages == 1);
      fail_unless(p->size == 512);

      FIXPage* p1 = fix_parser_alloc_page(parser, 500);
      fail_unless(p1 != NULL);
      fail_unless(parser->used_pages == 2);
      fail_unless(p->size == 512);

      FIXPage* p2 = fix_parser_alloc_page(parser, 513);
      fail_unless(p2 == NULL);
      fail_unless(parser->error.code == FIX_ERROR_TOO_BIG_PAGE);
      fail_unless(parser->used_pages == 2);

      fix_parser_free(parser);
   }
}
END_TEST

Suite* make_fix_parser_tests_suite()
{
   Suite* s = suite_create("fix_parser");
   TCase* tc_core = tcase_create("Core");
   tcase_add_test(tc_core, CreateParserTest);
   tcase_add_test(tc_core, SetErrorParserTest);
   tcase_add_test(tc_core, GetFreePageTest);
   tcase_add_test(tc_core, MaxPagesTest);
   tcase_add_test(tc_core, MaxGroupsTest);
   tcase_add_test(tc_core, MaxPageSizeTest);
   suite_add_tcase(s, tc_core);
   return s;
}
