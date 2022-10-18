use std::io::{self, Read};

fn main() {
    let flag : i32 = 13371337;

    loop {
        println!("The value of flag is = {}", flag);
		let mut reader = io::stdin();
        reader.read(&mut [0; 10]).unwrap();
    }
}