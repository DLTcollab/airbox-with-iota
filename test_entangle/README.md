# Create MAM payload in RTL8195

Adapting test_mam of [entangled](https://github.com/iotaledger/entangled) in RTL8195

* Execution time : 134.576196 s
* Result : ![](https://i.imgur.com/Ly3S45N.png)
* Revised content:
  * Replace Unity with other functions, e.g. memcmp().
  * Change file paths.
  * Run single thread instead of multi thread.
  * Dynamic memory allocation for some arrays.
  * Replace Recursion with loops iteration.
  
* Issues:
  * Stack overflow after testing.
  * MAM parse
  
