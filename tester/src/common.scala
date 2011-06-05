import java.io.{File, FileInputStream, PrintStream}
import scala.util.Sorting.quickSort

package common {
  object Util {
    def isDir(file: File) = file.isDirectory

    def matchPattern(s: String): (File => Boolean) = dir => (if (s == "_") ".*" else s).r.findPrefixOf(dir.getName).isDefined
  }
       
  class DirWrapper(dir: File, toMake: Boolean = false) {
    if (toMake)
      dir.mkdir()
    else
      assert(Util.isDir(dir), println(dir.getAbsolutePath))

    def this(dw: DirWrapper, name: String) = this({
      val f = dw / name
      if (!f.exists)
        f.mkdir()
      f
    })

    val name = dir.getName
    val absolutePath = dir.getAbsolutePath

    def subdirs(filter: (File => Boolean)): Iterable[DirWrapper] = {
      val res = dir.listFiles.filter(file => Util.isDir(file) && filter(file))
      quickSort(res)(new Ordering[File] {
        override def compare(x: File, y: File) = x.getName compare y.getName
      })
      res.map(file => new DirWrapper(file))
    }

    def / (name: String) : File = new File(dir, name)
  }

  package details {
    abstract sealed class SpecialSymbol
    object feed extends SpecialSymbol
    object endl extends SpecialSymbol

    abstract sealed class Color
    object black extends Color  // тут под black подразумевается стандартный для
                                // консоли цвет, то есть для моего ноутбука -- белый
    object green extends Color
    object red extends Color
    object blue extends Color
  }

  class StreamWrapper(out: PrintStream) {
    def << (s: String) : StreamWrapper = {
      out.print(s)
      return this
    }

    def << (i: Int) : StreamWrapper = {
      out.print(i)
      return this
    }

    def << (d: Double) : StreamWrapper = {
      out.print(d)
      return this
    }

    def << (s: details.SpecialSymbol) : StreamWrapper = {
      s match {
        case details.feed => out.println(" \\")
        case details.endl => out.println()
      }
      return this
    }

    def << (c: details.Color) : StreamWrapper = {
      out.print(c match {
        case details.black  => "\033[0;37m"
        case details.red    => "\033[0;31m"
        case details.green  => "\033[0;32m"
        case details.blue   => "\033[0;34m"
      })
      return this
    }

    def << (f: File) : StreamWrapper = {
      out.print(f.getAbsolutePath)
      return this
    }

    def << (dir: DirWrapper) : StreamWrapper = {
      out.print(dir.absolutePath)
      return this
    }
  }

  class Attributes(in: File) {
    private[this] val props = new java.util.Properties()
    props.load(new FileInputStream(in))

    def apply(s: String) : String = props.getProperty(s)
  }
}
