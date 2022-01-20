use clap::Parser;
use std::path::{PathBuf, Path};
use serde::Deserialize;
use std::error::Error;
use std::collections::HashMap;
use std::fs::File;
use std::io::{self, Write, BufWriter};
use std::fmt::{self, Display, Formatter};

#[derive(Debug, Clone, Parser)]
struct Arguments {
    #[clap(help = "Path to the configuration file")]
    config_path: PathBuf,
    #[clap(help = "Path to the output directory for C++ source files")]
    output_directory: PathBuf,
}

#[derive(Debug, Copy, Clone, Deserialize)]
#[serde(rename_all = "lowercase")]
enum SelfKind {
    Ref,
    Mut
}

impl SelfKind {
    fn as_ptr(self) -> ArgType {
        match self {
            Self::Ref => ArgType::ConstPtr,
            Self::Mut => ArgType::MutPtr,
        }
    }
}

#[derive(Debug, Copy, Clone, Deserialize)]
#[serde(rename_all = "snake_case")]
enum ArgType {
    U8,
    U16,
    U32,
    U64,
    Usize,
    I8,
    I16,
    I32,
    I64,
    Bool,
    ConstPtr,
    MutPtr,
}

impl Display for ArgType {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Self::U8 => write!(f, "uint8_t"),
            Self::U16 => write!(f, "uint16_t"),
            Self::U32 => write!(f, "uint32_t"),
            Self::U64 => write!(f, "uint64_t"),
            Self::Usize => write!(f, "size_t"),
            Self::I8 => write!(f, "int8_t"),
            Self::I16 => write!(f, "int16_t"),
            Self::I32 => write!(f, "int32_t"),
            Self::I64 => write!(f, "int64_t"),
            Self::Bool => write!(f, "bool"),
            Self::ConstPtr => write!(f, "const void*"),
            Self::MutPtr => write!(f, "void*"),
        }
    }
}

#[derive(Debug, Clone, Deserialize)]
struct Argument {
    name: String,
    #[serde(rename = "type")]
    arg_type: ArgType,
}

#[derive(Debug, Clone, Deserialize)]
struct Function {
    #[serde(rename = "self")]
    self_kind: SelfKind,
    args: Vec<Argument>,
    #[serde(rename = "return")]
    return_type: ArgType,
}

#[derive(Debug, Clone, Deserialize)]
struct FunctionMap(HashMap<String, Function>);

#[derive(Debug, Clone, Deserialize)]
struct TypeMap(HashMap<String, FunctionMap>);

fn read_config<P: AsRef<Path>>(path: P) -> Result<TypeMap, Box<dyn Error>> {
    Ok(serde_json::from_reader(File::open(path)?)?)
}

fn mangle_str(source: &str, destination: &mut String) {
    for b in source.bytes() {
        if b.is_ascii_alphanumeric() {
            destination.push(b as char);
        } else {
            let hex = b"0123456789abcdef";
            destination.push('_');
            destination.push(hex[(b & 0xF) as usize] as char);
            destination.push(hex[(b >> 4) as usize] as char);
        }
    }
}

fn mangle(type_name: &str, func_name: &str) -> String {
    let mut mangled = String::from("_rustcxx_binding_");
    mangle_str(type_name, &mut mangled);
    mangled.push_str("_m_");
    mangle_str(func_name, &mut mangled);
    mangled
}

fn generate_source(config: &TypeMap, out: &mut impl Write) -> io::Result<()> {
    for (type_name, func_map) in config.0.iter() {
        for (func_name, func) in func_map.0.iter() {
            let mangled_func_name = mangle(type_name, func_name);
            write!(out, "extern \"C\" {} {}(", func.return_type, mangled_func_name)?;
            write!(out, "{} self", func.self_kind.as_ptr())?;
            for arg in func.args.iter() {
                write!(out, ", {} {}", arg.arg_type, arg.name)?;
            }
            writeln!(out, ");")?;
        }
        writeln!(out, "")?;
    }
    
    for (type_name, func_map) in config.0.iter() {
        let (namespace, type_basename) = type_name.rsplit_once("::").unwrap();
        writeln!(out, "namespace {} {{", namespace)?;
        writeln!(out, "class {}Ref {{", type_basename)?;
        writeln!(out, "public:")?;
        for (func_name, func) in func_map.0.iter() {
            write!(out, "{} {}(", func.return_type, func_name)?;
            for (i, arg) in func.args.iter().enumerate() {
                let maybe_comma = if i == 0 { "" } else { ", " };
                write!(out, "{}{} {}", maybe_comma, arg.arg_type, arg.name)?;
            }
            let maybe_const = match func.self_kind {
                SelfKind::Ref => " const",
                SelfKind::Mut => "",
            };
            writeln!(out, "){};", maybe_const)?;
        }
        writeln!(out, "private:")?;
        writeln!(out, "    {}Ref() = delete;", type_basename)?;
        writeln!(out, "    void* m_ptr;")?;
        writeln!(out, "}}")?;
        writeln!(out, "}} // namespace {}", namespace)?;
        writeln!(out, "")?;
    }
    Ok(())
}

fn main() -> Result<(), Box<dyn Error>> {
    let arguments = Arguments::parse();
    let config = read_config(&arguments.config_path)?;
    let mut out_src_file_path = arguments.output_directory.clone();
    out_src_file_path.push("out.cpp");
    let mut out_src_file = BufWriter::new(File::create(out_src_file_path)?);
    println!("{:#?}", &config);
    generate_source(&config, &mut out_src_file)?;
    Ok(())
}
