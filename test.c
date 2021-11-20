
struct test_t {
  i32 b;
  i32 a[3];
};

fn test(test_t *a)
{
  a->a[2] = 3;
}

fn main()
{
  test_t a;
  
  test(&a);
}
