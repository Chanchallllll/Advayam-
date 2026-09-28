"use client";

import React, { useEffect, useState, useRef } from "react";
import Header from "@/components/Header";
import Footer from "@/components/Footer";
import { useAuth } from "@/context/AuthContext";
import { apiFetch } from "@/lib/api";
import { useToast } from "@/context/ToastContext";
import {
  Users,
  Mail,
  UserCheck,
  Shield,
  Search,
  Loader,
  RefreshCw,
  Bold,
  Italic,
  Underline,
  Heading3,
  Link2,
  List,
  CornerDownLeft,
  Database,
  ArrowUpRight,
  LifeBuoy,
  Calendar,
  AlertTriangle,
  CheckCircle,
  FileText
} from "lucide-react";

interface UserRecord {
  _id: string;
  name: string;
  email: string;
  role: string;
  isVerified: boolean;
  createdAt: string;
}

interface SubscriberRecord {
  _id: string;
  email: string;
  subscribedAt: string;
}

interface TicketRecord {
  _id: string;
  user?: {
    _id: string;
    name: string;
    email: string;
    role: string;
  };
  category: string;
  title: string;
  description: string;
  status: 'open' | 'in-progress' | 'resolved';
  priority: 'low' | 'medium' | 'high';
  attachments: {
    name: string;
    size: number;
    type: string;
    data: string; // base64
  }[];
  createdAt: string;
}

export default function AdminDashboard() {
  const { user, isLoading: authLoading } = useAuth();
  const { showToast } = useToast();

  const [users, setUsers] = useState<UserRecord[]>([]);
  const [subscribers, setSubscribers] = useState<SubscriberRecord[]>([]);
  const [tickets, setTickets] = useState<TicketRecord[]>([]);
  const [dataLoading, setDataLoading] = useState(true);
  const [error, setError] = useState("");

  const [activeTab, setActiveTab] = useState<"users" | "newsletter" | "tickets">("users");
  const [userSearch, setUserSearch] = useState("");
  const [newsletterSearch, setNewsletterSearch] = useState("");
  const [ticketSearch, setTicketSearch] = useState("");

  const [newsletterSubject, setNewsletterSubject] = useState("");
  const [newsletterContent, setNewsletterContent] = useState("");
  const [sendingNewsletter, setSendingNewsletter] = useState(false);
  const [sendSuccessMessage, setSendSuccessMessage] = useState("");
  const [sendErrorMessage, setSendErrorMessage] = useState("");

  const [ticketReplies, setTicketReplies] = useState<{ [key: string]: string }>({});
  const [replyingToId, setReplyingToId] = useState<string | null>(null);
  const [isSendingReply, setIsSendingReply] = useState(false);

  const textareaRef = useRef<HTMLTextAreaElement>(null);

  const insertTag = (openTag: string, closeTag: string = "") => {
    const textarea = textareaRef.current;
    if (!textarea) return;

    const start = textarea.selectionStart;
    const end = textarea.selectionEnd;
    const text = textarea.value;
    const selectedText = text.substring(start, end);
    const replacement = openTag + selectedText + closeTag;

    setNewsletterContent(
      text.substring(0, start) + replacement + text.substring(end)
    );

    setTimeout(() => {
      textarea.focus();
      textarea.setSelectionRange(
        start + openTag.length,
        start + openTag.length + selectedText.length
      );
    }, 10);
  };

  const handleAddLink = () => {
    const url = prompt("Enter URL:", "https://");
    if (url) {
      insertTag(`<a href="${url}" style="color: #BE5209; text-decoration: underline;" target="_blank">`, "</a>");
    }
  };

  const handleSendNewsletter = async (e: React.FormEvent) => {
    e.preventDefault();
    setSendSuccessMessage("");
    setSendErrorMessage("");

    if (!newsletterSubject.trim() || !newsletterContent.trim()) {
      setSendErrorMessage("Please enter both a subject and some content.");
      showToast("Please enter both a subject and some content.", "error");
      return;
    }

    setSendingNewsletter(true);
    try {
      const response = await apiFetch("/newsletter/send", {
        method: "POST",
        body: JSON.stringify({
          subject: newsletterSubject,
          content: newsletterContent,
        }),
      });

      const successMsg = response.message || "Newsletter sent successfully!";
      setSendSuccessMessage(successMsg);
      showToast(successMsg, "success");
      setNewsletterSubject("");
      setNewsletterContent("");
    } catch (err: any) {
      const errorMsg = err.message || "Failed to send newsletter email.";
      setSendErrorMessage(errorMsg);
      showToast(errorMsg, "error");
    } finally {
      setSendingNewsletter(false);
    }
  };

  const handleSendReply = async (ticketId: string) => {
    const msg = ticketReplies[ticketId] || "";
    if (!msg.trim()) {
      showToast("Please enter a reply message.", "error");
      return;
    }

    setIsSendingReply(true);
    try {
      await apiFetch(`/tickets/${ticketId}/reply`, {
        method: "POST",
        body: JSON.stringify({ message: msg }),
      });
      showToast("Reply email sent to user successfully!", "success");
      setTicketReplies((prev) => ({ ...prev, [ticketId]: "" }));
      setReplyingToId(null);
      // Refresh tickets to reflect 'in-progress' status
      fetchAdminData(false);
    } catch (err: any) {
      showToast(err.message || "Failed to send reply email.", "error");
    } finally {
      setIsSendingReply(false);
    }
  };

  const handleUpdateTicket = async (ticketId: string, updates: { status?: string; priority?: string }) => {
    try {
      const updated = await apiFetch<TicketRecord>(`/tickets/${ticketId}`, {
        method: "PATCH",
        body: JSON.stringify(updates),
      });
      setTickets((prev) => prev.map((t) => (t._id === ticketId ? updated : t)));
      showToast("Ticket updated successfully!", "success");
    } catch (err: any) {
      showToast(err.message || "Failed to update ticket.", "error");
    }
  };

  useEffect(() => {
    if (!authLoading && user && user.role === "admin") {
      fetchAdminData(false);
    }
  }, [user, authLoading]);

  const fetchAdminData = async (showToastOnSuccess = false) => {
    setDataLoading(true);
    setError("");
    try {
      const [fetchedUsers, fetchedSubscribers, fetchedTickets] = await Promise.all([
        apiFetch<UserRecord[]>("/users"),
        apiFetch<SubscriberRecord[]>("/newsletter/subscriptions"),
        apiFetch<TicketRecord[]>("/tickets"),
      ]);
      setUsers(fetchedUsers || []);
      setSubscribers(fetchedSubscribers || []);
      setTickets(fetchedTickets || []);
      if (showToastOnSuccess) {
        showToast("Database records synced successfully!", "success");
      }
    } catch (err: any) {
      console.error(err);
      const msg = err.message || "Failed to load admin data.";
      setError(msg);
      showToast(msg, "error");
    } finally {
      setDataLoading(false);
    }
  };

  if (authLoading) {
    return (
      <main className="flex min-h-screen flex-col bg-[#ECECEC] text-[#202020]">
        <Header />
        <div className="flex-1 flex items-center justify-center pt-32">
          <Loader className="h-8 w-8 animate-spin text-[#BE5209]" />
        </div>
        <Footer />
      </main>
    );
  }

  if (!user || user.role !== "admin") {
    return (
      <main className="flex min-h-screen flex-col bg-[#ECECEC] text-[#202020]">
        <Header />
        <section className="flex-1 flex flex-col items-center justify-center px-6 pt-32 pb-20">
          <div className="max-w-md text-center p-8 bg-white border-2 border-black rounded-[2rem] shadow-md">
            <h1 className="text-3xl font-black text-red-600">Access Denied</h1>
            <p className="mt-4 font-jakarta text-sm font-medium text-slate-600">
              You must be logged in as an administrator to view this page.
            </p>
            <a
              href="/auth/login"
              className="mt-6 inline-flex h-12 items-center justify-center rounded-xl bg-[#BE5209] px-6 text-sm font-semibold text-white hover:bg-black transition"
            >
              Go to Login
            </a>
          </div>
        </section>
        <Footer />
      </main>
    );
  }

  const filteredUsers = users.filter(
    (u) =>
      (u.name || "").toLowerCase().includes(userSearch.toLowerCase()) ||
      u.email.toLowerCase().includes(userSearch.toLowerCase()) ||
      u.role.toLowerCase().includes(userSearch.toLowerCase())
  );

  const filteredSubscribers = subscribers.filter((s) =>
    s.email.toLowerCase().includes(newsletterSearch.toLowerCase())
  );

  const filteredTickets = tickets.filter(
    (t) =>
      t.title.toLowerCase().includes(ticketSearch.toLowerCase()) ||
      t.category.toLowerCase().includes(ticketSearch.toLowerCase()) ||
      (t.user?.name || "").toLowerCase().includes(ticketSearch.toLowerCase()) ||
      (t.user?.email || "").toLowerCase().includes(ticketSearch.toLowerCase())
  );

  const totalUsers = users.length;
  const verifiedUsersCount = users.filter((u) => u.isVerified).length;
  const developerUsersCount = users.filter((u) => u.role === "developer").length;
  const totalSubscribers = subscribers.length;
  const totalTickets = tickets.length;
  const openTicketsCount = tickets.filter((t) => t.status === "open").length;

  return (
    <main className="flex min-h-screen flex-col bg-[#F3F4F6] text-[#202020]">
      <Header />

      <section className="px-6 pb-24 pt-40 flex-1">
        <div className="mx-auto max-w-8xl">
          {/* Header Area */}
          <div className="flex flex-col md:flex-row md:items-center md:justify-between gap-6 border-b border-gray-200 pb-8">
            <div>
              <div className="flex items-center gap-3">
                <span className="h-2.5 w-2.5 rounded-full bg-green-500 animate-pulse" />
                <span className="text-xs font-bold uppercase tracking-wider text-slate-400">Security Level: Admin</span>
              </div>
              <h1 className="text-4xl md:text-5xl font-black font-jakarta tracking-tight mt-1">
                Admin <span className="bg-gradient-to-r from-[#BE5209] to-[#E37A24] bg-clip-text text-transparent">Control Center</span>
              </h1>
              <p className="mt-2 text-sm font-medium text-slate-500 max-w-2xl">
                Real-time portal to manage registered accounts, audit user verification states, inspect developers, and resolve tickets.
              </p>
            </div>

            <button
              onClick={() => fetchAdminData(true)}
              disabled={dataLoading}
              className="inline-flex h-11 items-center justify-center rounded-xl bg-white border border-gray-200 shadow-sm px-5 font-bold text-sm text-[#202020] hover:bg-slate-50 active:scale-95 transition-all disabled:opacity-50 gap-2 cursor-pointer"
            >
              <RefreshCw className={`h-4 w-4 text-[#BE5209] ${dataLoading ? "animate-spin" : ""}`} />
              <span>Sync Records</span>
            </button>
          </div>

          {error && (
            <div className="mt-8 rounded-2xl border border-red-200 bg-red-50 p-4 text-sm font-bold text-red-600 shadow-sm">
              {error}
            </div>
          )}

          {/* Premium Glass Metrics Grid */}
          <div className="mt-8 grid gap-6 grid-cols-1 sm:grid-cols-2 lg:grid-cols-4">
            {/* Metric 1 */}
            <div className="bg-white border border-gray-200 rounded-2xl p-6 shadow-sm hover:-translate-y-1 hover:shadow-md transition-all duration-300 group">
              <div className="flex items-center justify-between">
                <span className="text-xs font-bold text-slate-400 uppercase tracking-wider">Total Registers</span>
                <div className="h-10 w-10 rounded-xl bg-orange-50 flex items-center justify-center text-[#BE5209] group-hover:scale-110 transition-transform">
                  <Users className="h-5 w-5" />
                </div>
              </div>
              <div className="mt-4 flex items-baseline gap-2">
                <span className="text-3xl font-black tracking-tight">{totalUsers}</span>
                <span className="text-xs font-semibold text-slate-400">accounts</span>
              </div>
            </div>

            {/* Metric 2 */}
            <div className="bg-white border border-gray-200 rounded-2xl p-6 shadow-sm hover:-translate-y-1 hover:shadow-md transition-all duration-300 group">
              <div className="flex items-center justify-between">
                <span className="text-xs font-bold text-slate-400 uppercase tracking-wider">Open Support Requests</span>
                <div className="h-10 w-10 rounded-xl bg-red-50 flex items-center justify-center text-red-600 group-hover:scale-110 transition-transform">
                  <LifeBuoy className="h-5 w-5" />
                </div>
              </div>
              <div className="mt-4 flex items-baseline gap-2">
                <span className="text-3xl font-black tracking-tight text-red-600">{openTicketsCount}</span>
                <span className="text-xs font-semibold text-slate-400">/ {totalTickets} total</span>
              </div>
            </div>

            {/* Metric 3 */}
            <div className="bg-white border border-gray-200 rounded-2xl p-6 shadow-sm hover:-translate-y-1 hover:shadow-md transition-all duration-300 group">
              <div className="flex items-center justify-between">
                <span className="text-xs font-bold text-slate-400 uppercase tracking-wider">Developer Base</span>
                <div className="h-10 w-10 rounded-xl bg-blue-50 flex items-center justify-center text-blue-600 group-hover:scale-110 transition-transform">
                  <Shield className="h-5 w-5" />
                </div>
              </div>
              <div className="mt-4 flex items-baseline gap-2">
                <span className="text-3xl font-black tracking-tight text-blue-600">{developerUsersCount}</span>
                <span className="text-xs font-semibold text-slate-400">enrolled</span>
              </div>
            </div>

            {/* Metric 4 */}
            <div className="bg-white border border-gray-200 rounded-2xl p-6 shadow-sm hover:-translate-y-1 hover:shadow-md transition-all duration-300 group">
              <div className="flex items-center justify-between">
                <span className="text-xs font-bold text-slate-400 uppercase tracking-wider">Subscriptions</span>
                <div className="h-10 w-10 rounded-xl bg-purple-50 flex items-center justify-center text-purple-600 group-hover:scale-110 transition-transform">
                  <Mail className="h-5 w-5" />
                </div>
              </div>
              <div className="mt-4 flex items-baseline gap-2">
                <span className="text-3xl font-black tracking-tight text-purple-600">{totalSubscribers}</span>
                <span className="text-xs font-semibold text-slate-400">emails</span>
              </div>
            </div>
          </div>

          {/* Main Content Card Panel */}
          <div className="mt-12 bg-white border border-gray-200 rounded-[2rem] overflow-hidden shadow-sm">
            {/* Sliding Pill Tab bar */}
            <div className="flex flex-wrap items-center gap-2 p-3 bg-slate-50 border-b border-gray-100">
              <button
                onClick={() => setActiveTab("users")}
                className={`inline-flex items-center gap-2.5 px-6 py-3 rounded-xl font-jakarta text-sm font-black transition-all cursor-pointer ${activeTab === "users"
                    ? "bg-white text-[#BE5209] shadow-sm border border-gray-150"
                    : "text-slate-400 hover:text-slate-600"
                  }`}
              >
                <Database className="h-4 w-4" />
                <span>User Registry ({totalUsers})</span>
              </button>

              <button
                onClick={() => setActiveTab("tickets")}
                className={`inline-flex items-center gap-2.5 px-6 py-3 rounded-xl font-jakarta text-sm font-black transition-all cursor-pointer ${activeTab === "tickets"
                    ? "bg-white text-[#BE5209] shadow-sm border border-gray-150"
                    : "text-slate-400 hover:text-slate-600"
                  }`}
              >
                <LifeBuoy className="h-4 w-4" />
                <span>Support Tickets ({totalTickets})</span>
              </button>

              <button
                onClick={() => setActiveTab("newsletter")}
                className={`inline-flex items-center gap-2.5 px-6 py-3 rounded-xl font-jakarta text-sm font-black transition-all cursor-pointer ${activeTab === "newsletter"
                    ? "bg-white text-[#BE5209] shadow-sm border border-gray-150"
                    : "text-slate-400 hover:text-slate-600"
                  }`}
              >
                <Mail className="h-4 w-4" />
                <span>Newsletter Hub ({totalSubscribers})</span>
              </button>
            </div>

            {/* Tab Contents */}
            <div className="p-8">
              {dataLoading && (
                <div className="py-24 flex flex-col items-center justify-center">
                  <Loader className="h-8 w-8 animate-spin text-[#BE5209]" />
                  <span className="mt-4 text-sm font-bold text-slate-400">Accessing database...</span>
                </div>
              )}

              {!dataLoading && activeTab === "users" && (
                <div className="animate-fadeIn">
                  {/* Search and Filters */}
                  <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4">
                    <div className="flex h-11 w-full max-w-md items-center rounded-xl border border-gray-200 bg-slate-50 px-4 focus-within:border-[#BE5209] focus-within:bg-white focus-within:shadow-sm transition-all">
                      <Search className="h-4 w-4 text-slate-400 shrink-0" />
                      <input
                        type="text"
                        placeholder="Search records by email, name, role..."
                        value={userSearch}
                        onChange={(e) => setUserSearch(e.target.value)}
                        className="ml-3 w-full bg-transparent text-sm font-medium outline-none text-[#202020]"
                      />
                    </div>
                  </div>

                  {/* Users Table */}
                  <div className="mt-6 overflow-x-auto border border-gray-100 rounded-2xl shadow-sm">
                    <table className="w-full text-left border-collapse">
                      <thead>
                        <tr className="border-b border-gray-100 text-slate-400 text-xs font-black uppercase tracking-wider bg-slate-50">
                          <th className="py-4.5 px-6">Name</th>
                          <th className="py-4.5 px-6">Email Address</th>
                          <th className="py-4.5 px-6">Assigned Role</th>
                          <th className="py-4.5 px-6">Verification</th>
                          <th className="py-4.5 px-6">Registration Date</th>
                        </tr>
                      </thead>
                      <tbody>
                        {filteredUsers.length > 0 ? (
                          filteredUsers.map((u) => (
                            <tr key={u._id} className="border-b border-gray-50 hover:bg-slate-50/50 font-semibold text-sm transition-colors">
                              <td className="py-4 px-6 text-[#202020] font-black">{u.name || "—"}</td>
                              <td className="py-4 px-6 text-slate-500 font-jakarta">{u.email}</td>
                              <td className="py-4 px-6">
                                <span className={`inline-flex items-center px-3 py-1 rounded-full text-xs font-bold border ${u.role === "admin"
                                    ? "bg-red-50 text-red-600 border-red-150"
                                    : u.role === "developer"
                                      ? "bg-blue-50 text-blue-600 border-blue-150"
                                      : "bg-slate-50 text-slate-600 border-gray-200"
                                  }`}>
                                  {u.role}
                                </span>
                              </td>
                              <td className="py-4 px-6">
                                <span className={`inline-flex items-center px-3 py-1 rounded-full text-xs font-bold border ${u.isVerified
                                    ? "bg-emerald-50 text-emerald-600 border-emerald-150"
                                    : "bg-amber-50 text-amber-600 border-amber-150"
                                  }`}>
                                  {u.isVerified ? "Verified" : "Pending OTP"}
                                </span>
                              </td>
                              <td className="py-4 px-6 text-slate-400 font-jakarta font-medium text-xs">
                                {new Date(u.createdAt).toLocaleDateString("en-US", {
                                  year: "numeric",
                                  month: "short",
                                  day: "numeric",
                                })}
                              </td>
                            </tr>
                          ))
                        ) : (
                          <tr>
                            <td colSpan={5} className="py-16 text-center text-sm font-semibold text-slate-400">
                              No matching accounts found.
                            </td>
                          </tr>
                        )}
                      </tbody>
                    </table>
                  </div>
                </div>
              )}

              {!dataLoading && activeTab === "tickets" && (
                <div className="animate-fadeIn space-y-6">
                  {/* Search bar */}
                  <div className="flex h-11 w-full max-w-md items-center rounded-xl border border-gray-200 bg-slate-50 px-4 focus-within:border-[#BE5209] focus-within:bg-white focus-within:shadow-sm transition-all">
                    <Search className="h-4 w-4 text-slate-400 shrink-0" />
                    <input
                      type="text"
                      placeholder="Search tickets by title, category, user..."
                      value={ticketSearch}
                      onChange={(e) => setTicketSearch(e.target.value)}
                      className="ml-3 w-full bg-transparent text-sm font-medium outline-none text-[#202020]"
                    />
                  </div>

                  {/* Tickets Cards List */}
                  <div className="grid gap-6 grid-cols-1">
                    {filteredTickets.length > 0 ? (
                      filteredTickets.map((t) => (
                        <div key={t._id} className="bg-slate-50 border border-gray-200 rounded-2xl p-6 shadow-sm flex flex-col gap-4">
                          {/* Card Header Info */}
                          <div className="flex flex-col md:flex-row md:items-center justify-between gap-4 border-b border-gray-200 pb-4">
                            <div>
                              <div className="flex flex-wrap items-center gap-2">
                                <span className="text-[11px] font-black uppercase text-slate-400 tracking-wider font-jakarta">{t.category}</span>
                                <span className="text-slate-300">•</span>
                                <div className="flex items-center gap-1 text-slate-400 text-xs font-semibold">
                                  <Calendar className="h-3.5 w-3.5" />
                                  <span>{new Date(t.createdAt).toLocaleString("en-US", { dateStyle: "medium", timeStyle: "short" })}</span>
                                </div>
                              </div>
                              <h3 className="text-xl font-black font-jakarta text-[#202020] mt-1">{t.title}</h3>
                              {t.user && (
                                <p className="text-xs text-slate-500 font-medium mt-1">
                                  Raised by <span className="font-bold text-[#BE5209]">{t.user.name || "Unknown"}</span> ({t.user.email})
                                </p>
                              )}
                            </div>

                            {/* Status and Priority Action select dropdowns */}
                            <div className="flex items-center gap-3 self-start md:self-center">
                              <div>
                                <label className="block text-[9px] font-black uppercase tracking-wider text-slate-400">Priority</label>
                                <select
                                  value={t.priority}
                                  onChange={(e) => handleUpdateTicket(t._id, { priority: e.target.value as any })}
                                  className={`mt-1 h-9 rounded-lg border text-xs font-bold px-2.5 outline-none bg-white ${t.priority === "high"
                                      ? "text-red-600 border-red-200 bg-red-50/50"
                                      : t.priority === "medium"
                                        ? "text-amber-600 border-amber-200 bg-amber-50/50"
                                        : "text-blue-600 border-blue-200 bg-blue-50/50"
                                    }`}
                                >
                                  <option value="low">Low</option>
                                  <option value="medium">Medium</option>
                                  <option value="high">High</option>
                                </select>
                              </div>

                              <div>
                                <label className="block text-[9px] font-black uppercase tracking-wider text-slate-400">Status</label>
                                <select
                                  value={t.status}
                                  onChange={(e) => handleUpdateTicket(t._id, { status: e.target.value as any })}
                                  className={`mt-1 h-9 rounded-lg border text-xs font-bold px-2.5 outline-none bg-white ${t.status === "resolved"
                                      ? "text-green-600 border-green-200 bg-green-50/50"
                                      : t.status === "in-progress"
                                        ? "text-[#BE5209] border-orange-200 bg-orange-50/50"
                                        : "text-slate-600 border-gray-200 bg-slate-100/50"
                                    }`}
                                >
                                  <option value="open">Open</option>
                                  <option value="in-progress">In Progress</option>
                                  <option value="resolved">Resolved</option>
                                </select>
                              </div>
                            </div>
                          </div>

                          {/* Ticket HTML Description Box */}
                          <div className="bg-white border border-gray-150 rounded-xl p-4 text-sm text-[#202020] leading-relaxed font-jakarta">
                            <span className="block text-[10px] font-black uppercase tracking-widest text-slate-400 mb-2 border-b border-gray-50 pb-1.5">Issue Description</span>
                            <div
                              className="prose prose-sm max-w-none text-[#202020] [&_strong]:font-bold [&_ul]:list-disc [&_ul]:pl-5 [&_a]:text-[#BE5209] [&_a]:underline"
                              dangerouslySetInnerHTML={{ __html: t.description }}
                            />
                          </div>

                          {/* Attachment / File Previews */}
                          {t.attachments && t.attachments.length > 0 && (
                            <div className="bg-white border border-gray-150 rounded-xl p-4">
                              <span className="block text-[10px] font-black uppercase tracking-widest text-slate-400 mb-3 border-b border-gray-50 pb-1.5">
                                Uploaded Media & Documents ({t.attachments.length})
                              </span>
                              <div className="flex flex-wrap gap-4">
                                {t.attachments.map((file, idx) => {
                                  const isImage = file.type.startsWith("image/");
                                  return (
                                    <div key={idx} className="flex items-center gap-3 border border-gray-200 rounded-xl p-2.5 max-w-xs w-full bg-slate-50 hover:bg-slate-100/50 transition-colors">
                                      {isImage ? (
                                        <img
                                          src={file.data}
                                          alt={file.name}
                                          className="h-10 w-10 object-cover rounded-lg border border-gray-250 cursor-zoom-in shrink-0"
                                          onClick={() => window.open(file.data, "_blank")}
                                        />
                                      ) : (
                                        <div className="h-10 w-10 rounded-lg bg-red-50 border border-red-100 flex items-center justify-center text-red-500 shrink-0">
                                          <FileText className="h-5 w-5" />
                                        </div>
                                      )}
                                      <div className="overflow-hidden flex-1 min-w-0">
                                        <span className="block text-xs font-bold truncate text-slate-700">{file.name}</span>
                                        <span className="block text-[10px] text-slate-400 font-jakarta font-semibold">
                                          {(file.size / 1024).toFixed(1)} KB
                                        </span>
                                      </div>
                                      <a
                                        href={file.data}
                                        download={file.name}
                                        className="h-8 w-8 rounded-lg bg-white border border-gray-200 flex items-center justify-center text-slate-400 hover:text-[#BE5209] hover:border-orange-200 shadow-sm transition-all"
                                        title="Download attachment"
                                      >
                                        <ArrowUpRight className="h-4 w-4" />
                                      </a>
                                    </div>
                                  );
                                })}
                              </div>
                            </div>
                          )}

                          {/* Reply Box Section */}
                          <div className="border-t border-gray-200/50 pt-4 flex flex-col gap-3">
                            {replyingToId === t._id ? (
                              <div className="flex flex-col gap-3 animate-fadeIn">
                                <textarea
                                  placeholder="Type your response email here... (The user will receive this response directly in their mailbox)"
                                  value={ticketReplies[t._id] || ""}
                                  onChange={(e) => setTicketReplies(prev => ({ ...prev, [t._id]: e.target.value }))}
                                  className="w-full h-28 rounded-xl border border-gray-200 bg-white p-3 font-jakarta text-xs outline-none focus:border-[#BE5209] resize-none text-[#202020] leading-relaxed shadow-inner"
                                />
                                <div className="flex items-center gap-3 justify-end">
                                  <button
                                    onClick={() => setReplyingToId(null)}
                                    className="h-9 px-4 rounded-lg bg-slate-100 hover:bg-slate-200 text-xs font-bold text-slate-650 transition cursor-pointer"
                                  >
                                    Cancel
                                  </button>
                                  <button
                                    onClick={() => handleSendReply(t._id)}
                                    disabled={isSendingReply || !(ticketReplies[t._id] || "").trim()}
                                    className="h-9 px-4 rounded-lg bg-[#BE5209] hover:bg-black text-xs font-bold text-white transition disabled:opacity-50 inline-flex items-center gap-1.5 cursor-pointer"
                                  >
                                    {isSendingReply ? (
                                      <>
                                        <Loader className="h-3 w-3 animate-spin text-white" />
                                        <span>Sending...</span>
                                      </>
                                    ) : (
                                      <span>Send Response</span>
                                    )}
                                  </button>
                                </div>
                              </div>
                            ) : (
                              <button
                                onClick={() => setReplyingToId(t._id)}
                                className="h-10 border border-[#BE5209] border-dashed rounded-xl flex items-center justify-center gap-2 text-xs font-bold text-[#BE5209] hover:bg-orange-50 active:scale-[0.99] transition cursor-pointer"
                              >
                                <Mail className="h-3.5 w-3.5" />
                                <span>Reply to Submitter via Email</span>
                              </button>
                            )}
                          </div>
                        </div>
                      ))
                    ) : (
                      <div className="py-20 text-center border border-dashed border-gray-200 rounded-2xl bg-white text-sm font-semibold text-slate-400">
                        No support tickets found matching your query.
                      </div>
                    )}
                  </div>
                </div>
              )}

              {!dataLoading && activeTab === "newsletter" && (
                <div className="grid grid-cols-1 lg:grid-cols-3 gap-8 animate-fadeIn">
                  {/* Broadcast Newsletter Card */}
                  <div className="bg-slate-50 border border-gray-200 rounded-2xl p-6 h-fit shadow-sm">
                    <div className="flex items-center justify-between border-b border-gray-200 pb-3">
                      <div>
                        <h3 className="font-jakarta text-base font-black text-[#202020]">
                          Mailing Broadcast
                        </h3>
                        <p className="text-[11px] text-slate-400 font-bold">
                          Send custom HTML mail to subscribers
                        </p>
                      </div>
                      <ArrowUpRight className="h-5 w-5 text-slate-400 shrink-0" />
                    </div>

                    <form onSubmit={handleSendNewsletter} className="mt-6 space-y-5">
                      <div>
                        <label className="block text-[11px] font-black uppercase text-slate-400 tracking-wider">
                          Email Subject
                        </label>
                        <input
                          type="text"
                          value={newsletterSubject}
                          onChange={(e) => setNewsletterSubject(e.target.value)}
                          placeholder="e.g. Advayam IDE v1.0.0 is Live!"
                          className="mt-2 h-11 w-full rounded-xl border border-gray-200 bg-white px-3 font-jakarta text-sm outline-none focus:border-[#BE5209] transition-all text-[#202020]"
                          required
                        />
                      </div>

                      <div>
                        <label className="block text-[11px] font-black uppercase text-slate-400 tracking-wider">
                          Newsletter Body
                        </label>

                        {/* Editor Toolbar */}
                        <div className="mt-2 flex flex-wrap gap-1 border border-gray-200 bg-white p-1.5 rounded-t-xl border-b-0">
                          <button
                            type="button"
                            onClick={() => insertTag("<strong>", "</strong>")}
                            title="Bold"
                            className="p-1.5 hover:bg-slate-100 rounded text-slate-500 hover:text-black transition"
                          >
                            <Bold className="h-3.5 w-3.5" />
                          </button>
                          <button
                            type="button"
                            onClick={() => insertTag("<em>", "</em>")}
                            title="Italic"
                            className="p-1.5 hover:bg-slate-100 rounded text-slate-500 hover:text-black transition"
                          >
                            <Italic className="h-3.5 w-3.5" />
                          </button>
                          <button
                            type="button"
                            onClick={() => insertTag("<u>", "</u>")}
                            title="Underline"
                            className="p-1.5 hover:bg-slate-100 rounded text-slate-500 hover:text-black transition"
                          >
                            <Underline className="h-3.5 w-3.5" />
                          </button>
                          <div className="w-px h-5 bg-black/10 self-center mx-1" />
                          <button
                            type="button"
                            onClick={() => insertTag("<h3>", "</h3>")}
                            title="Heading"
                            className="p-1.5 hover:bg-slate-100 rounded text-slate-500 hover:text-black transition"
                          >
                            <Heading3 className="h-3.5 w-3.5" />
                          </button>
                          <button
                            type="button"
                            onClick={handleAddLink}
                            title="Add Link"
                            className="p-1.5 hover:bg-slate-100 rounded text-slate-500 hover:text-black transition"
                          >
                            <Link2 className="h-3.5 w-3.5" />
                          </button>
                          <button
                            type="button"
                            onClick={() => insertTag("<ul>\n  <li>", "</li>\n</ul>")}
                            title="Bullet List"
                            className="p-1.5 hover:bg-slate-100 rounded text-slate-500 hover:text-black transition"
                          >
                            <List className="h-3.5 w-3.5" />
                          </button>
                          <button
                            type="button"
                            onClick={() => insertTag("<br />\n")}
                            title="Line Break"
                            className="p-1.5 hover:bg-slate-100 rounded text-slate-500 hover:text-black transition"
                          >
                            <CornerDownLeft className="h-3.5 w-3.5" />
                          </button>
                        </div>

                        <textarea
                          ref={textareaRef}
                          value={newsletterContent}
                          onChange={(e) => setNewsletterContent(e.target.value)}
                          placeholder="<p>Hello world!</p><p>We are excited to announce...</p>"
                          rows={9}
                          className="w-full rounded-b-xl border border-gray-200 bg-white p-3 font-jakarta text-sm outline-none focus:border-[#BE5209] resize-none border-t-0 text-[#202020] leading-relaxed"
                          required
                        />
                      </div>

                      {sendSuccessMessage && (
                        <p className="text-xs font-bold text-emerald-600 bg-emerald-50 border border-emerald-200 rounded-xl p-3.5">
                          {sendSuccessMessage}
                        </p>
                      )}

                      {sendErrorMessage && (
                        <p className="text-xs font-bold text-red-600 bg-red-50 border border-red-200 rounded-xl p-3.5">
                          {sendErrorMessage}
                        </p>
                      )}

                      <button
                        type="submit"
                        disabled={sendingNewsletter}
                        className="w-full h-11 rounded-xl bg-[#BE5209] text-white font-bold text-sm hover:bg-black transition-all active:scale-95 disabled:opacity-50 gap-2 inline-flex items-center justify-center cursor-pointer shadow-sm"
                      >
                        {sendingNewsletter ? (
                          <>
                            <Loader className="h-4 w-4 animate-spin text-white" />
                            <span>Broadcasting...</span>
                          </>
                        ) : (
                          <span>Send Broadcast</span>
                        )}
                      </button>
                    </form>
                  </div>

                  {/* Newsletter List */}
                  <div className="lg:col-span-2 space-y-6">
                    {/* Search Bar */}
                    <div className="flex h-11 max-w-md items-center rounded-xl border border-gray-200 bg-slate-50 px-4 focus-within:border-[#BE5209] focus-within:bg-white focus-within:shadow-sm transition-all">
                      <Search className="h-4 w-4 text-slate-400 shrink-0" />
                      <input
                        type="text"
                        placeholder="Search active subscriptions..."
                        value={newsletterSearch}
                        onChange={(e) => setNewsletterSearch(e.target.value)}
                        className="ml-3 w-full bg-transparent text-sm font-medium outline-none text-[#202020]"
                      />
                    </div>

                    {/* Subscribers Table */}
                    <div className="overflow-x-auto bg-white border border-gray-100 rounded-2xl shadow-sm">
                      <table className="w-full text-left border-collapse">
                        <thead>
                          <tr className="border-b border-gray-100 text-slate-400 text-xs font-black uppercase tracking-wider bg-slate-50">
                            <th className="py-4.5 px-6">Email Address</th>
                            <th className="py-4.5 px-6">Date Subscribed</th>
                          </tr>
                        </thead>
                        <tbody>
                          {filteredSubscribers.length > 0 ? (
                            filteredSubscribers.map((s) => (
                              <tr key={s._id} className="border-b border-gray-50 hover:bg-slate-50/50 font-semibold text-sm transition-colors">
                                <td className="py-4 px-6 font-black text-[#202020]">{s.email}</td>
                                <td className="py-4 px-6 text-slate-400 font-jakarta font-medium text-xs">
                                  {new Date(s.subscribedAt).toLocaleDateString("en-US", {
                                    year: "numeric",
                                    month: "short",
                                    day: "numeric",
                                    hour: "2-digit",
                                    minute: "2-digit",
                                  })}
                                </td>
                              </tr>
                            ))
                          ) : (
                            <tr>
                              <td colSpan={2} className="py-16 text-center text-sm font-semibold text-slate-400">
                                No newsletter subscribers found.
                              </td>
                            </tr>
                          )}
                        </tbody>
                      </table>
                    </div>
                  </div>
                </div>
              )}
            </div>
          </div>
        </div>
      </section>

      <Footer />
    </main>
  );
}
