import java.io.{File, FileInputStream, PrintStream}
import scala.util.Sorting.quickSort

package object common {
  def isDir(file: File) = file.isDirectory

  def matchPattern(s: String): (File => Boolean) = dir => (if (s == "_") ".*" else s).r.findPrefixOf(dir.getName).isDefined
}

package common {
  class DirWrapper(dir: File, toMake: Boolean = false) {
    if (toMake)
      dir.mkdir()
    else
      assert(isDir(dir), println(dir.getAbsolutePath))

    def this(dw: DirWrapper, name: String) = this({
      val f = dw / name
      if (!f.exists)
        f.mkdir()
      f
    })

    val name = dir.getName

    def subdirs(filter: (File => Boolean)): Iterable[DirWrapper] = {
      val res = dir.listFiles.filter(file => isDir(file) && filter(file))
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
  }

  class StreamWrapper(out: PrintStream) {
    def << (s: String) : StreamWrapper = {
      out.print(s)
      return this
    }

    def << (s: details.SpecialSymbol) : StreamWrapper = {
      s match {
        case details.feed => out.println(" \\")
        case details.endl => out.println()
      }
      return this
    }

    def << (f: File) : StreamWrapper = {
      out.print(f.getAbsolutePath)
      return this
    }
  }

  class Attributes(in: File) {
    private[this] val props = new java.util.Properties()
    props.load(new FileInputStream(in))

    def apply(s: String) : String = props.getProperty(s)
  }
}
