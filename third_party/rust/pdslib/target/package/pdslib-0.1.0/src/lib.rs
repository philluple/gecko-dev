pub mod budget;
pub mod events;
pub mod mechanisms;
pub mod pds;
pub mod queries;

pub fn hello() {
  println!("Hello from pdslib!");
}

#[cfg(test)]
mod tests {
  use super::*;

  #[test]
  fn test_hello() {
      hello();
      assert_eq!(2 + 2, 4);
  }
}
