mod bindings;
mod datatype;
mod function;
mod mangle;
mod separator;

use crate::bindings::Bindings;
use crate::function::{Method, SelfKind};
use crate::mangle::Mangle;
use crate::separator::Separator;
use clap::Parser;
use std::error::Error;
use std::fs::File;
use std::io::{self, BufWriter, Write};
use std::path::{Path, PathBuf};

#[derive(Debug, Clone, Parser)]
struct Arguments {
    #[clap(help = "Path to the configuration file")]
    config_path: PathBuf,
    #[clap(help = "Path to the output directory for C++ source files")]
    output_directory: PathBuf,
}

fn read_config<P: AsRef<Path>>(path: P) -> Result<Bindings, Box<dyn Error>> {
    Ok(serde_json::from_reader(File::open(path)?)?)
}

fn check(config: &Bindings) -> Result<(), Box<dyn Error>> {
    for (_type_name, type_bindings) in config.0.iter() {
        for (_func_name, func) in type_bindings.methods.iter() {
            func.return_type.check()?;
            for arg in func.args.iter() {
                arg.arg_type.check()?;
            }
        }
    }
    Ok(())
}

fn generate_cpp(config: &Bindings, out: &mut impl Write) -> io::Result<()> {
    for (type_name, type_bindings) in config.0.iter() {
        for (func_name, func) in type_bindings.methods.iter() {
            let mangled_func_name = Method {
                method_name: func_name.clone(),
                type_name: type_name.clone(),
            }
            .mangle()
            .unwrap();
            let mut sep = Separator::new(", ");
            let mut sep_fn = sep.as_fn_mut();

            write!(
                out,
                "extern \"C\" {} {}(",
                func.return_type.c_types()[0],
                mangled_func_name,
            )?;
            write!(
                out,
                "{}",
                func.self_kind
                    .as_ptr()
                    .view_c_types(&mut sep_fn, "self")
                    .unwrap()
            )?;
            for arg in func.args.iter() {
                write!(
                    out,
                    "{}",
                    arg.arg_type.view_c_types(&mut sep_fn, &arg.name).unwrap()
                )?;
            }
            writeln!(out, ");")?;
        }
        writeln!(out, "")?;
    }

    for (type_name, type_bindings) in config.0.iter() {
        writeln!(out, "namespace {} {{", type_name.namespace())?;
        writeln!(out, "class {} final {{", type_name.base())?;
        writeln!(out, "public:")?;

        let type_basename = type_name.base();
        writeln!(out, "    {this}() = delete;", this = type_basename)?;
        writeln!(
            out,
            "    {this}(const {this}& rhs) = delete;",
            this = type_basename
        )?;
        writeln!(
            out,
            "    {this}& operator=(const {this}& rhs) = delete;",
            this = type_basename
        )?;

        for (func_name, func) in type_bindings.methods.iter() {
            let mut sep = Separator::new(", ");
            let mut sep_fn = sep.as_fn_mut();

            write!(out, "    {} {}(", func.return_type.cxx_type(), func_name)?;
            for arg in func.args.iter() {
                write!(out, "{}{} {}", sep_fn(), arg.arg_type.cxx_type(), arg.name)?;
            }
            let maybe_const = match func.self_kind {
                SelfKind::Ref => " const",
                SelfKind::Mut => "",
            };
            writeln!(out, "){};", maybe_const)?;
        }

        writeln!(out, "")?;
        writeln!(out, "private:")?;
        writeln!(out, "    explicit {this}(void* ptr);", this = type_basename)?;
        writeln!(out, "    void* m_ptr;")?;
        writeln!(out, "}};")?;
        writeln!(out, "}} // namespace {}", type_name.namespace())?;
        writeln!(out, "")?;
    }
    Ok(())
}

fn inner_main() -> Result<(), Box<dyn Error>> {
    let arguments = Arguments::parse();
    let config = read_config(&arguments.config_path)?;
    check(&config)?;

    let mut out_src_file_path = arguments.output_directory.clone();
    out_src_file_path.push("out.cpp");
    let mut out_src_file = BufWriter::new(File::create(out_src_file_path)?);
    println!("{:#?}", &config);
    generate_cpp(&config, &mut out_src_file)?;
    Ok(())
}

fn main() {
    if let Err(e) = inner_main() {
        eprintln!("Error: {}", e);
        std::process::exit(1);
    }
}
