
fn print(i32 n)
{
  asm("
    lbp
    ldr
    int 0
  ");
}

fn main()
{
  i32 i, p, n, prime[16];
  
  n = 16;
  
  i = 0;
  while (i < n) {
    prime[i] = 1;
    i = i + 1;
  }
  
  p = 2;
  while (p * p < n) {
    if (prime[p] > 0) {
      i = p * p;
      while (i < n) {
        prime[i] = 0;
        i = i + p;
      }
    }
    
    p = p + 1;
  }
  
  i = 0;
  while (i < n) {
    if (prime[i] > 0)
      print(i);
    i = i + 1;
  }
}
